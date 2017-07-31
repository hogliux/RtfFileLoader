/*
  ==============================================================================

    JUCE Attributed String helper
    Copyright 2017 ROLI Ltd.

  ==============================================================================
*/

#if defined(__OBJC__)
 #define JUCE_CORE_INCLUDE_NATIVE_HEADERS 1
#endif

#include "AttributedStringSerializer.h"

#if (! (JUCE_IOS || JUCE_MAC) || defined(__OBJC__))

static void addTextToXml (XmlElement& parent, const String& str)
{
    int n = str.length();
    int len = 0;

    for (int pos = 0; pos < n; pos += len)
    {
        int bpos = str.indexOfChar (pos, L'\n');
        len = (bpos >= 0 ? bpos - pos : n - pos);

        if (len  > 0) parent.addTextElement (str.substring (pos, pos + len));

        if (bpos >= 0)
        {
            parent.createNewChildElement ("br");
            pos++;
        }
    }
}

void AttributedStringSerializer::writeAttributedStringToOutputStream (const AttributedString& attrStr, OutputStream& stream)
{
    static Identifier sizeIdentifier   ("size");
    static Identifier familyIdentifier ("family");
    static Identifier syleIdentifier   ("style");
    static Identifier colourIdentifier ("colour");

    XmlElement root ("JUCE");

    const int n = attrStr.getNumAttributes();
    int pos = 0;

    const String& str = attrStr.getText();

    for (int i = 0; i < n; ++i)
    {
        const AttributedString::Attribute& attr = attrStr.getAttribute (i);

        if (pos < attr.range.getStart())
        {
            addTextToXml (root, str.substring (pos, attr.range.getStart()));
            pos = attr.range.getStart();
        }

        XmlElement* font = root.createNewChildElement ("font");

        font->setAttribute (sizeIdentifier,   static_cast<double> (attr.font.getHeight()));
        font->setAttribute (familyIdentifier, attr.font.getTypefaceName());

        StringArray styles;

        const int styleFlags = attr.font.getStyleFlags();

        if ((styleFlags & Font::bold)       != 0) styles.add ("bold");
        if ((styleFlags & Font::italic)     != 0) styles.add ("italic");
        if ((styleFlags & Font::underlined) != 0) styles.add ("underlined");

        if (styles.size() > 0)
            font->setAttribute (syleIdentifier, styles.joinIntoString (","));

        String red   (String::toHexString ((int) attr.colour.getRed()));
        String green (String::toHexString ((int) attr.colour.getGreen()));
        String blue  (String::toHexString ((int) attr.colour.getBlue()));

        if (red  .length() < 2) red   = String ("0") + red;
        if (green.length() < 2) green = String ("0") + green;
        if (blue .length() < 2) blue  = String ("0") + blue;

        font->setAttribute (colourIdentifier, String ("#") + red + green + blue);
        addTextToXml (*font, str.substring (attr.range.getStart(), attr.range.getEnd()));

        pos = attr.range.getEnd() + 1;
    }

    root.writeToStream (stream, "");
}

AttributedString* AttributedStringSerializer::createAttributedStringFromInputStream (InputStream& stream)
{
    static Identifier sizeIdentifier   ("size");
    static Identifier familyIdentifier ("family");
    static Identifier syleIdentifier   ("style");
    static Identifier colourIdentifier ("colour");

    XmlDocument xmlDocument (stream.readString());

    ScopedPointer<XmlElement> xml = xmlDocument.getDocumentElement();
    if (xml != nullptr)
    {
        ScopedPointer<AttributedString> result = new AttributedString;

        forEachXmlChildElement (*xml, xmlElement)
        {
            if (xmlElement->isTextElement())
            {
                result->append (xmlElement->getText());
            }
            else if (xmlElement->hasTagName ("br"))
            {
                result->append ("\n");
            }
            else if (xmlElement->hasTagName ("font"))
            {
                const float  size   = static_cast<float> (xmlElement->getDoubleAttribute (sizeIdentifier, 12.0));
                const String family = xmlElement->getStringAttribute (familyIdentifier, "");
                const StringArray styleTags = StringArray::fromTokens (xmlElement->getStringAttribute (syleIdentifier, ""), ",", "");
                const String colourString = xmlElement->getStringAttribute (colourIdentifier, "");

                int styleFlags = 0;

                if (styleTags.contains ("bold"))       styleFlags |= Font::bold;
                if (styleTags.contains ("italic"))     styleFlags |= Font::italic;
                if (styleTags.contains ("underlined")) styleFlags |= Font::underlined;

                Colour colour = Colours::transparentBlack;

                if (colourString.startsWith ("#") && colourString.length() == 7)
                {
                    const String red   (colourString.substring (1, 3));
                    const String green (colourString.substring (3, 5));
                    const String blue  (colourString.substring (5, 7));

                    colour = Colour ((uint8) red.getHexValue32(), (uint8) green.getHexValue32(), (uint8) blue.getHexValue32());
                }

                forEachXmlChildElement (*xmlElement, textElement)
                {
                    String subtext;

                    if      (textElement->isTextElement())   subtext += textElement->getText();
                    else if (textElement->hasTagName ("br")) subtext += "\n";

                    if      (colour.getAlpha() != 0 && family.isNotEmpty()) result->append (subtext, Font (family, size, styleFlags), colour);
                    else if (colour.getAlpha() != 0)                        result->append (subtext, colour);
                    else if (                          family.isNotEmpty()) result->append (subtext, Font (family, size, styleFlags));
                    else                                                    result->append (subtext);
                }
            }
        }

        return result.release();
    }

    return nullptr;
}

AttributedString* AttributedStringSerializer::createAttributedStringFromFile (const File& inputFile)
{
    ScopedPointer<InputStream> inputStream = inputFile.createInputStream();

    if (inputStream != nullptr)
        return createAttributedStringFromInputStream (*inputStream);

    return nullptr;
}

void AttributedStringSerializer::writeAttributedStringToFile (const AttributedString& str, const File& outFile)
{
    ScopedPointer<OutputStream> outputStream = outFile.createOutputStream();

    if (outputStream != nullptr)
        return writeAttributedStringToOutputStream (str, *outputStream);
}

#if (JUCE_MAC || JUCE_IOS)
AttributedString* AttributedStringSerializer::createAttributedStringFromRTFFile (const File& rtfFile)
{
    ScopedPointer<InputStream> inputStream = rtfFile.createInputStream();

    if (inputStream != nullptr)
        return createAttributedStringFromRTFData (*inputStream);

    return nullptr;
}

AttributedString* AttributedStringSerializer::createAttributedStringFromRTFData (InputStream& inputStream)
{
    MemoryBlock rtfData;

    if (inputStream.readIntoMemoryBlock (rtfData) <= 0)
        return nullptr;



    if (NSData* nsRTFData = [[NSData alloc] initWithBytes:rtfData.getData() length:rtfData.getSize()])
    {
        NSAttributedString* attr = [[NSAttributedString alloc] initWithRTF:nsRTFData documentAttributes:nullptr];
        [nsRTFData release];

        if (attr != nullptr)
        {
            ScopedPointer<AttributedString> juceAttr = new AttributedString;

            NSUInteger pos = 0, n = [attr length];

            NSRange range;


            NSString* nsText = [attr string];

            while (pos < n)
            {
                NSDictionary<NSString*, id>* attribute = [attr attributesAtIndex:pos
                                                           longestEffectiveRange:&range
                                                                         inRange:NSMakeRange (pos, n - pos)];

                if (attribute == nullptr)
                    range = NSMakeRange (pos, n - pos);

                ScopedPointer<Font> font;
                ScopedPointer<Colour> colour;

                if (attribute != nullptr)
                {
                    if (NSObject* obj = [attribute objectForKey:NSFontAttributeName])
                    {
                        if ([obj isKindOfClass:[NSFont class]] == YES)
                        {
                            if (NSFontDescriptor* fontDescriptor = [((NSFont*)obj) fontDescriptor])
                            {
                                if (NSObject* obj2 = [fontDescriptor objectForKey:NSFontFamilyAttribute])
                                {
                                    if ([obj2 isKindOfClass:[NSString class]])
                                    {
                                        if (const char* fontFamily = [((NSString*)obj2) cStringUsingEncoding:NSUTF8StringEncoding])
                                        {
                                            NSFontSymbolicTraits traits = [fontDescriptor symbolicTraits];
                                            int style = ((traits & NSFontItalicTrait) != 0 ? Font::italic : 0)
                                            | ((traits & NSFontBoldTrait)   != 0 ? Font::bold : 0);

                                            if (NSObject* underline = [attribute objectForKey:NSUnderlineStyleAttributeName])
                                            {
                                                if ([underline isKindOfClass:[NSNumber class]])
                                                {
                                                    if ([((NSNumber*)underline) boolValue] == YES)
                                                        style |= Font::underlined;
                                                }
                                            }


                                            font = new Font (Font (fontFamily, 12, style).withPointHeight ([fontDescriptor pointSize]));
                                        }
                                    }
                                }
                            }
                        }
                    }

                    if (NSObject* nsColour = [attribute objectForKey:NSForegroundColorAttributeName])
                    {
                        float r = [((NSColor*)nsColour) redComponent];
                        float g = [((NSColor*)nsColour) greenComponent];
                        float b = [((NSColor*)nsColour) blueComponent];

                        colour = new Colour (Colour::fromFloatRGBA (r, g, b, 1.0f));
                    }
                }

                String text = juce::CharPointer_UTF8 ([[nsText substringWithRange:range] cStringUsingEncoding:NSUTF8StringEncoding]);

                if      (font != nullptr && colour != nullptr) juceAttr->append (text, *font, *colour);
                else if (font != nullptr)                      juceAttr->append (text, *font);
                else if                    (colour != nullptr) juceAttr->append (text, *colour);
                else                                           juceAttr->append (text);

                pos = range.location + range.length;
            }

            [attr release];
            return juceAttr.release();
        }
    }

    return nullptr;
}
#endif

#endif
