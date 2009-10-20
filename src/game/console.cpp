#include "console.h"
#include "util/bitmap.h"
#include "util/font.h"
#include "util/funcs.h"
#include "util/file-system.h"
#include "input/input-manager.h"
#include "input/input-map.h"
#include "globals.h"
#include <string>
#include <sstream>

using namespace std;

ConsoleEnd Console::endl;
const std::string Console::DEFAULT_FONT = Global::DEFAULT_FONT;

Console::Console(const int maxHeight, const std::string & font):
state(Closed),
maxHeight(maxHeight),
height(0),
font(font),
textHeight(15),
textWidth(15),
offset(0){
    const int delay = 10;
    input.set(Keyboard::Key_TILDE, delay * 2, false, '~');
    input.set(Keyboard::Key_A, delay, false, 'a');
    input.set(Keyboard::Key_BACKSPACE, delay, false, 9);
    input.set(Keyboard::Key_ESC, delay, false, 8);
}

Console::~Console(){
}
    
void Console::act(){
    double speed = 10.0;
    switch (state){
        case Closed : {
            break;
        }
        case Open : {
            break;
        }
        case Closing : {
            int distance = height;
            height -= (int)((double)distance / speed + 1.0);
            if (height <= 0){
                height = 0;
                state = Closed;
            }
            break;
        }
        case Opening : {
            int distance = maxHeight - height;
            height += (int)((double)distance / speed + 1.0);
            if (height >= maxHeight){
                height = maxHeight;
                state = Open;
            }
            break;
        }
    }
    checkStream();
}
    
bool Console::doInput() throw (ReturnException) {
    InputMap<char>::Output inputState = InputManager::getMap(input);
    if (inputState['~']){
        toggle();
        return false;
    }

    if (inputState['a']){
        currentCommand << 'a';
    }

    if (inputState[8]){
        throw ReturnException();
    }

    if (inputState[9]){
        backspace();
    }

    return true;
}

void Console::draw(const Bitmap & work){
    /* if we can show something */
    if (height > 0){
        Bitmap::drawingMode(Bitmap::MODE_TRANS);
	Bitmap::transBlender(0, 0, 0, 160);
        work.rectangleFill(0, 0, work.getWidth(), height, Bitmap::makeColor(200,0,0));
        work.horizontalLine(0, height, work.getWidth(), Bitmap::makeColor(200, 200, 200));
        const Font & font = Font::getFont(Filesystem::find(getFont()), textWidth, textHeight);
        //font.printf(0, height - font.getHeight(), Bitmap::makeColor(255, 255, 255), work, "Console!", 0 );
        Bitmap::drawingMode(Bitmap::MODE_SOLID);
	// if (state == Open){
            if (!lines.empty()){
                int start = height - font.getHeight() * 2;
                for (std::vector<std::string>::reverse_iterator i = lines.rbegin(); i != lines.rend() && start > 0; ++i){
                    std::string str = *i;
                    font.printf(0, start, Bitmap::makeColor(255,255,255), work, str, 0);
                    start -= font.getHeight();
                }
            }
            font.printf(0, height - font.getHeight(), Bitmap::makeColor(255,255,255), work, "> " + currentCommand.str(), 0);
        // }
    }
}
    
void Console::toggle(){
    switch (state){
        case Open:
        case Opening: {
            state = Closing;
            break;
        }
        case Closed:
        case Closing: {
            state = Opening;
            break;
        }
    }
}

void Console::backspace(){
    string now = currentCommand.str();
    now = now.substr(0, now.size()-1);
    currentCommand.str(now);
    currentCommand.rdbuf()->pubseekoff(0, ios_base::end, ios_base::out);
    currentCommand.clear();
}
    
Console & Console::operator<<(const ConsoleEnd & e){
    checkStream();
    return *this;
}

void Console::clear(){
    lines.clear();
    textInput.str("");
    textInput.clear();
}

std::stringstream & Console::add(){
    checkStream();
    return textInput;
}

void Console::checkStream(){
    if (!textInput.str().empty()){
	lines.push_back(textInput.str());
	textInput.str("");
    }
}
