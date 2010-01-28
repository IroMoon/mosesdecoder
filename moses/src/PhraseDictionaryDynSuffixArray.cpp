#include "PhraseDictionaryDynSuffixArray.h"
#include "DynSAInclude/utils.h"
#include "FactorCollection.h"

namespace Moses {
  PhraseDictionaryDynSuffixArray::PhraseDictionaryDynSuffixArray(size_t numScoreComponent):
    PhraseDictionary(numScoreComponent) { 
    srcSA_ = 0; 
    trgSA_ = 0;
    srcCrp_ = new vector<wordID_t>();
    trgCrp_ = new vector<wordID_t>();
    vocab_ = new Vocab(false);
  }
  PhraseDictionaryDynSuffixArray::~PhraseDictionaryDynSuffixArray(){
    delete srcSA_;
    delete trgSA_;
    delete vocab_;
    delete srcCrp_;
    delete trgCrp_;
  }
bool PhraseDictionaryDynSuffixArray::Load(string source, string target, string alignments
																					, const std::vector<float> &weight
																					, size_t tableLimit
																					, const LMList &languageModels
																					, float weightWP) 
{

	m_weight = weight;
	m_tableLimit = tableLimit;
	m_languageModels = &languageModels;
	m_weightWP = weightWP;

	loadCorpus(new FileHandler(source), *srcCrp_, srcSntBreaks_);
  loadCorpus(new FileHandler(target), *trgCrp_, trgSntBreaks_);
  //assert(srcSntBreaks_.size() == trgSntBreaks_.size());
  std::cerr << "Vocab: " << std::endl;
  vocab_->printVocab();
  // build suffix arrays and auxilliary arrays
  srcSA_ = new DynSuffixArray(srcCrp_); 
  if(!srcSA_) return false;
  //trgSA_ = new DynSuffixArray(trgCrp_); 
  //if(!trgSA_) return false;
  loadAlignments(new FileHandler(alignments));
	LoadVocabLookup();
	
  return true;
}
int PhraseDictionaryDynSuffixArray::loadAlignments(FileHandler* align) {
}
void PhraseDictionaryDynSuffixArray::LoadVocabLookup()
{
	FactorCollection &factorCollection = FactorCollection::Instance();
	
	Vocab::Word2Id::const_iterator iter;
	for (iter = vocab_->vocabStart(); iter != vocab_->vocabEnd(); ++iter)
	{
		const word_t &str = iter->first;
		wordID_t arrayId = iter->second;
		const Factor *factor = factorCollection.AddFactor(Input, 0, str, false);
		vocabLookup_[factor] = arrayId;
		vocabLookupRev_[arrayId] = factor;
	}
			
}
void PhraseDictionaryDynSuffixArray::InitializeForInput(const InputType& input)
{
  /*assert(m_runningNodesVec.size() == 0);
  size_t sourceSize = input.GetSize();
  m_runningNodesVec.resize(sourceSize);
  for (size_t ind = 0; ind < m_runningNodesVec.size(); ++ind)
  {
    ProcessedRule *initProcessedRule = new ProcessedRule(m_collection);
    ProcessedRuleStack *processedStack = new ProcessedRuleStack(sourceSize - ind + 1);
    processedStack->Add(0, initProcessedRule); // init rule. stores the top node in tree
    m_runningNodesVec[ind] = processedStack;
  }*/
  return;
}
void PhraseDictionaryDynSuffixArray::CleanUp() {
  return;
}
void PhraseDictionaryDynSuffixArray::SetWeightTransModel(const std::vector<float, std::allocator<float> >&) {
  return;
}
int PhraseDictionaryDynSuffixArray::loadCorpus(FileHandler* corpus, vector<wordID_t>& cArray, 
    vector<wordID_t>& sntArray) {
  string line, word;
  int sntIdx(0);
  corpus->seekg(0);
  while(getline(*corpus, line)) {
    sntArray.push_back(sntIdx);
    std::istringstream ss(line.c_str());
    while(ss >> word) {
      ++sntIdx;
      cArray.push_back(vocab_->getWordID(word));
    }          
  }
  cArray.push_back(Vocab::kOOVWordID);  // signify end of corpus for ssarray
  return cArray.size();
}
	
const TargetPhraseCollection *PhraseDictionaryDynSuffixArray::GetTargetPhraseCollection(const Phrase& src) const {
	cerr << src << " ";
	TargetPhraseCollection *ret = new TargetPhraseCollection();
	size_t phraseSize = src.GetSize();
  vector<wordID_t> srcLocal(phraseSize), wrdIndices(0);  
	for (size_t pos = 0; pos < phraseSize; ++pos)
	{
		const Word &word = src.GetWord(pos);
		const Factor *factor = word.GetFactor(0);
		
		std::map<const Factor *, wordID_t>::const_iterator iterLookup;
		iterLookup = vocabLookup_.find(factor);
		
		if (iterLookup == vocabLookup_.end())
      return ret;
		else
		{
			wordID_t arrayId = iterLookup->second;
      srcLocal[pos] = arrayId;
			cerr << arrayId << " ";
		}
	}
  unsigned denom = srcSA_->countPhrase(&srcLocal, &wrdIndices);
  const int* sntIndexes = getSntIndexes(wrdIndices);	
	return ret;
} 
const int* PhraseDictionaryDynSuffixArray::getSntIndexes(vector<unsigned>& wrdIndices) const {
  vector<unsigned>::const_iterator vit;
  int* sntIndexes = new int[wrdIndices.size()];
  for(int i = 0; i < wrdIndices.size(); ++i) {
    vit = std::lower_bound(srcSntBreaks_.begin(), srcSntBreaks_.end(), wrdIndices[i]);
    sntIndexes[i] = unsigned(vit - srcSntBreaks_.begin());
  }
  return sntIndexes;
}
	
std::vector<PhrasePair> SentenceAlignment::Extract(int maxPhraseLength)
{
	std::vector<PhrasePair>	ret;
	
	int countE = english.size();
  int countF = foreign.size();
	
  // check alignments for english phrase startE...endE
  for(int startE=0;startE<countE;startE++) 
	{
    for(int endE=startE; 
				(endE<countE && endE<startE+maxPhraseLength);
				endE++) 
		{
      
      int minF = 9999;
      int maxF = -1;
      vector< int > usedF = alignedCountF;
      for(int ei=startE;ei<=endE;ei++) 
			{
				for(int i=0;i<alignedToE[ei].size();i++) 
				{
					int fi = alignedToE[ei][i];
					// cout << "point (" << fi << ", " << ei << ")\n";
					if (fi<minF) { minF = fi; }
					if (fi>maxF) { maxF = fi; }
					usedF[ fi ]--;
				} // for(int i=0;i<sentence
      } // for(int ei=startE
      
      // cout << "f projected ( " << minF << "-" << maxF << ", " << startE << "," << endE << ")\n"; 
			
      if (maxF >= 0 && // aligned to any foreign words at all
					maxF-minF < maxPhraseLength) 
			{ // foreign phrase within limits
				
				// check if foreign words are aligned to out of bound english words
				bool out_of_bounds = false;
				for(int fi=minF;fi<=maxF && !out_of_bounds;fi++)
				{
					if (usedF[fi]>0) 
					{
						// cout << "ouf of bounds: " << fi << "\n";
						out_of_bounds = true;
					}
				}
				
				// cout << "doing if for ( " << minF << "-" << maxF << ", " << startE << "," << endE << ")\n"; 
				if (!out_of_bounds)
				{
					// start point of foreign phrase may retreat over unaligned
					for(int startF=minF;
							(startF>=0 &&
							 startF>maxF-maxPhraseLength && // within length limit
							 (startF==minF || alignedCountF[startF]==0)); // unaligned
							startF--)
					{
						// end point of foreign phrase may advance over unaligned
						for (int endF=maxF;
								 (endF<countF && 
									endF<startF+maxPhraseLength && // within length limit
									(endF==maxF || alignedCountF[endF]==0)); // unaligned
								 endF++)
						{
							
							PhrasePair phrasePair(startE,endE,startF,endF);
							ret.push_back(phrasePair);
						} // for (int endF=maxF;
					}	// for(int startF=minF;
				} // if (!out_of_bounds)
      } // if (maxF >= 0 &&
    }
  }	
	
	return ret;
}

}// end namepsace
