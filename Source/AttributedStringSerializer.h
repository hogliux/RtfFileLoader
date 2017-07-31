/*
  ==============================================================================

    AttributedStringSerializer.h
    Created: 19 Apr 2017 2:30:50pm
    Author:  Fabian Renn-Giles

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

struct AttributedStringSerializer
{
    static AttributedString* createAttributedStringFromInputStream (InputStream& stream);
    static void writeAttributedStringToOutputStream (const AttributedString& str, OutputStream& stream);

    static AttributedString* createAttributedStringFromFile (const File& inputFile);
    static void writeAttributedStringToFile (const AttributedString& str, const File& outFile);

   #if (JUCE_MAC || JUCE_IOS)
    static AttributedString* createAttributedStringFromRTFData (InputStream& stream);
    static AttributedString* createAttributedStringFromRTFFile (const File& rtfFile);
   #endif
};
