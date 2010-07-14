#ifndef _paintown_menu_option_h
#define _paintown_menu_option_h

#include <string>
#include <vector>
#include "gui/box.h"
#include "util/load_exception.h"
#include "util/language-string.h"
#include "gui/context-box.h"

class Bitmap;
class Token;
namespace Menu {
    class Context;
}
namespace OldMenu{
    class Menu;
}
class Point;

namespace Gui{
    class Animation;
}

class MenuOption : public Gui::ContextItem {
public:
    // Do logic before run part
    virtual void logic()=0;

    // Do drawing or animations below text... If this is overridden need to cylce through the animations if useage is planned
    virtual void drawBelow(Bitmap *work);

    // do draw or animations above text
    virtual void drawAbove(Bitmap *work);

    // Finally it has been selected, this is what shall run 
    // endGame will be set true if it is a terminating option
    virtual void run(const Menu::Context &) = 0;

    // This is to pass paramaters to an option ie a bar or something
    // return true to pause key input
    virtual bool leftKey();
    virtual bool rightKey();

    // Reset animations
    virtual void resetAnimations();

    // Update animations This is called regardless and only when the option is active
    virtual void updateAnimations();

    //! Set parent
    virtual void setParent(OldMenu::Menu *menu);

    enum state
    {
        Selected = 0,
        Deselected,
        Run
    };

    enum OptionType{
        Event = 0,
        Option,
        AdjustableOption
    };

    MenuOption(Token *token, const OptionType t = Option) throw (LoadException);

    virtual ~MenuOption();

protected:

    //! This is the owner of this option
    OldMenu::Menu *parent;
private:
    state currentState;
    OptionType mType;
    LanguageString text;
    LanguageString infoText;
    Bitmap *bmp;
    int adjustLeftColor;
    int adjustRightColor;
    bool runnable;
    bool forRemoval;

    // Image resource for use in individual options
    std::vector<Gui::Animation *>backgroundAnimations;
    std::vector<Gui::Animation *>foregroundAnimations;
public:

    inline void setState(const state s) { currentState = s; }
    inline state getState() const { return currentState; }
    inline void setType(const OptionType t) { mType = t; }
    inline OptionType getType() const { return mType; }
    inline void setText(const LanguageString & t) { text = t; }
    virtual inline std::string getText() { return text.get(); }
    inline void setInfoText(const LanguageString &t) { infoText = t; }
    inline std::string getInfoText(){ return infoText.get(); }
    inline void setBitmap(Bitmap *b) { bmp = b; }
    inline Bitmap *getBitmap() const { return bmp; }
    inline void setLeftAdjustColor(int c) { adjustLeftColor = c; }
    inline int getLeftAdjustColor() const { return adjustLeftColor; }
    inline void setRightAdjustColor(int c) { adjustRightColor = c; }
    inline int getRightAdjustColor() const { return adjustRightColor; }
    inline void setRunnable(const bool r) { runnable = r; }
    inline bool isRunnable() const { return runnable; }
    inline void setForRemoval(const bool r) { forRemoval = r; }
    inline bool scheduledForRemoval() const { return forRemoval; }
    inline OldMenu::Menu *getParent() const { return parent; }

public:
    inline const std::string getName(){ return this->getText(); }
    inline bool isAdjustable(){ return (this->mType == AdjustableOption); }
    inline int getLeftColor(){ return this->adjustLeftColor; }
    inline int getRightColor(){ return this->adjustRightColor; }

protected:
    void readName(Token * token);
    void readInfo(Token * token);
};

#endif
