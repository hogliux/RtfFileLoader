/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/
#include "AttributedStringSerializer.h"

//==============================================================================
class RtfFileLoaderApplication  : public JUCEApplication
{
public:
    //==============================================================================
    RtfFileLoaderApplication() {}

    const String getApplicationName() override       { return ProjectInfo::projectName; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override       { return true; }

    //==============================================================================
    void initialise (const String& commandLine) override
    {
        // This method is where you should put your application's initialisation code..

        mainWindow = new MainWindow (getApplicationName());
    }

    void shutdown() override
    {
        // Add your application's shutdown code here..

        mainWindow = nullptr; // (deletes our window)
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        // This is called when the app is being asked to quit: you can ignore this
        // request and let the app carry on running, or call quit() to allow the app to close.
        quit();
    }

    void anotherInstanceStarted (const String& commandLine) override
    {
        // When another instance of the app is launched while this one is running,
        // this method is invoked, and the commandLine parameter tells you what
        // the other instance's command-line arguments were.
    }

    //==============================================================================
    /*
        This class implements the desktop window that contains an instance of
        our MainContentComponent class.
    */
    class MainWindow    : public DocumentWindow
    {
    public:

        class MainContentComponent : public Component, private Button::Listener
        {
        private:
            //==============================================================================
            //==============================================================================
            //==============================================================================
            class TextComponent : public Component, private ComponentListener
            {
            public:
                TextComponent ()
                {
                    setOpaque (true);
                    setSize (1, 1);

                    parentHierarchyChanged();
                }

                void setText (const AttributedString& attr)
                {
                    layout.createLayout (attr, getWidth());
                    setSize (getWidth(), layout.getHeight());

                    repaint();
                }

                void paint (Graphics& g) override
                {
                    g.fillAll (Colours::white);
                    layout.draw (g, getLocalBounds().toFloat());
                }

            private:
                void parentHierarchyChanged() override
                {
                    Viewport* newParent = findParentComponentOfClass <Viewport> ();
                    if (newParent != parent)
                    {
                        if (parent != nullptr)
                            parent->removeComponentListener (this);

                        parent = newParent;

                        if (parent != nullptr)
                        {
                            parent->addComponentListener (this);
                            componentMovedOrResized (*parent, false, true);
                        }
                    }
                }

                void componentMovedOrResized (Component&, bool, bool wasResized) override
                {
                    if (wasResized && parent != nullptr)
                        setSize (parent->getWidth(), getHeight());
                }

                Viewport* parent = nullptr;
                TextLayout layout;
            };

        public:

            //==============================================================================
            //==============================================================================
            //==============================================================================
            MainContentComponent()
            {
                setOpaque (true);

                viewport.setViewedComponent (&text, false);

                load.addListener (this);
                save.addListener (this);

                save.setEnabled (false);

               #if (JUCE_MAC || JUCE_IOS)
                loadFromRtf.addListener (this);
               #endif

                addAndMakeVisible (load);
                addAndMakeVisible (save);

               #if (JUCE_MAC || JUCE_IOS)
                addAndMakeVisible (loadFromRtf);
               #endif

                addAndMakeVisible (viewport);

                setSize (640, 480);
            }

            void resized() override
            {
                auto r = getLocalBounds();

                auto header = r.removeFromTop (40);

               #if (JUCE_MAC || JUCE_IOS)
                const int numHeaderButtons = 3;
               #else
                const int numHeaderButtons = 2;
               #endif

                auto width = header.getWidth() / numHeaderButtons;

                load.setBounds (header.removeFromLeft (width));
                save.setBounds (header.removeFromLeft (width));

               #if (JUCE_MAC || JUCE_IOS)
                loadFromRtf.setBounds (header.removeFromLeft (width));
               #endif

                viewport.setBounds (r);
            }

            void paint (Graphics& g) override
            {
                g.fillAll (Colours::white);
            }

        private:
            void buttonClicked (Button* btn) override
            {
                if (btn == &load)
                {
                    FileChooser fc ("Choose juce attributed string xml file");

                    if (fc.browseForFileToOpen ())
                    {
                        lastLoadedString = AttributedStringSerializer::createAttributedStringFromFile (fc.getResult());

                        if (lastLoadedString != nullptr)
                        {
                            text.setText (*lastLoadedString);
                            save.setEnabled (true);
                        }
                    }
                }
                else if (btn == &save)
                {
                    FileChooser fc ("Choose juce attributed string xml file");

                    if (lastLoadedString != nullptr && fc.browseForFileToSave (true))
                    {
                        if (fc.getResult().existsAsFile())
                            fc.getResult().deleteFile();

                        AttributedStringSerializer::writeAttributedStringToFile (*lastLoadedString, fc.getResult());
                    }
                }
               #if (JUCE_MAC || JUCE_IOS)
                else if (btn == &loadFromRtf)
                {
                    FileChooser fc ("Choose .rtf file");

                    if (fc.browseForFileToOpen ())
                    {
                        lastLoadedString = AttributedStringSerializer::createAttributedStringFromRTFFile (fc.getResult());

                        if (lastLoadedString != nullptr)
                        {
                            text.setText (*lastLoadedString);
                            save.setEnabled (true);
                        }
                    }
                }
               #endif
            }

            //==============================================================================
            TextButton load {"Load..."}, save {"Save..."};
           #if (JUCE_MAC || JUCE_IOS)
            TextButton loadFromRtf {"Load .rtf file..."};
           #endif
            Viewport viewport;
            TextComponent text;
            ScopedPointer<AttributedString> lastLoadedString;
        };

        MainWindow (String name)  : DocumentWindow (name,
                                                    Desktop::getInstance().getDefaultLookAndFeel()
                                                                          .findColour (ResizableWindow::backgroundColourId),
                                                    DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainContentComponent(), true);

            centreWithSize (getWidth(), getHeight());
            setVisible (true);
        }

        void closeButtonPressed() override
        {
            // This is called when the user tries to close this window. Here, we'll just
            // ask the app to quit when this happens, but you can change this to do
            // whatever you need.
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

        /* Note: Be careful if you override any DocumentWindow methods - the base
           class uses a lot of them, so by overriding you might break its functionality.
           It's best to do all your work in your content component instead, but if
           you really have to override any DocumentWindow methods, make sure your
           subclass also calls the superclass's method.
        */

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    ScopedPointer<MainWindow> mainWindow;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (RtfFileLoaderApplication)
