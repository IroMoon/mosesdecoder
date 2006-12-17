// $Id$
// vim:tabstop=2

/***********************************************************************
Moses - factored phrase-based language decoder
Copyright (C) 2006 University of Edinburgh

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
***********************************************************************/

#include "PhraseDictionary.h"
#include "StaticData.h"
#include "InputType.h"

PhraseDictionary::PhraseDictionary(size_t numScoreComponent, ScoreIndexManager &scoreIndexManager)
	: Dictionary(numScoreComponent),m_tableLimit(0)
{
	scoreIndexManager.AddScoreProducer(this);
}

PhraseDictionary::~PhraseDictionary() {}
	
const TargetPhraseCollection *PhraseDictionary::
GetTargetPhraseCollection(InputType const& src,WordsRange const& range) const 
{
	return GetTargetPhraseCollection(src.GetSubString(range));
}

const std::string PhraseDictionary::GetScoreProducerDescription(int idx) const
{
	return "Translation score, file=" + m_filePath;
}

size_t PhraseDictionary::GetNumScoreComponents() const
{
	return m_numScoreComponent;
}
