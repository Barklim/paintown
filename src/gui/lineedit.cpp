#include "util/bitmap.h"
#include "lineedit.h"
#include <iostream>
#include "globals.h"

#include "gui/keys.h"

using namespace Gui;

static std::ostream & debug( int level ){
	Global::debug( level ) << "[line edit] ";
	return Global::debug( level );
}

LineEdit::LineEdit() : 
currentSetFont(0), 
currentSetText(""),
hAlignment(T_Middle),
hAlignMod(T_Middle),
vAlignment(T_Middle),
inputTypeValue(inputGeneral),
changed_(0),
autoResizable(0),
textX(0),
textY(0),
cursorX(0),
cursorY(0),
cursorIndex(0),
textColor(0),
textSizeH(0),
limit(0),
blinkRate(500),
focused(false),
changeCounter(0)
{
	cursorTime.reset();
}

LineEdit::~LineEdit()
{
}
	
bool LineEdit::didChanged( unsigned long long & counter ){
	bool did = counter < changeCounter;
	counter = changeCounter;
	return did;
}

// If the font size changes
void LineEdit::fontChange()
{
	changed();
}

// Update
void LineEdit::act()
{
	if((blinkRate*2)<=cursorTime.msecs())cursorTime.reset();
	if(changed_)
	{
		textSizeH = currentSetFont->getHeight();
		if(autoResizable)
		{
			position.height = textSizeH+2;
			position.width = currentSetFont->textLength(currentSetText.c_str())+4;
		}
		else
		{
			if(hAlignMod==T_Left)
			{
				if(currentSetFont->textLength(currentSetText.c_str())>position.width)
				{
					hAlignment=T_Right;
				}
				else hAlignment=T_Left;
			}
		}
		switch(hAlignment)
		{
			case T_Left:
				textX = 2;
				cursorX = textX + currentSetFont->textLength(currentSetText.substr(0,cursorIndex).c_str()) + 1;
				break;
			case T_Middle:
				textX = (position.width/2) - (currentSetFont->textLength(currentSetText.c_str())/2);
				cursorX = (textX) + currentSetFont->textLength(currentSetText.substr(0,cursorIndex).c_str()) + 1;
				break;
			case T_Right:
				textX = position.width - currentSetFont->textLength(currentSetText.c_str());//(position.width - 1)-2;
				cursorX = position.width - currentSetFont->textLength(currentSetText.substr(0,currentSetText.length()-cursorIndex).c_str());
				break;
			case T_Bottom:
			case T_Top:
				break;
		}
		
		switch(vAlignment)
		{
			case T_Top:
				textY = 1;
				cursorY = 1;
				break;
			case T_Middle:
				textY = cursorY = (position.height - textSizeH-(5))/2;
				break;
			case T_Bottom:
				textY = (position.height - 1) - textSizeH - 1;
				cursorY = textY - textSizeH;
				break;
			case T_Right:
			case T_Left:
				break;
		}
			
		//textY++;
		//textX++;
		stable();
	}
}
		
// Draw
void LineEdit::render(const Bitmap & work){

	checkWorkArea();
	// Check if we are using a rounded box
	if(position.radius>0) {
		Bitmap::transBlender( 0, 0, 0, colors.bodyAlpha );
		roundRectFill( workArea, position.radius, 0, 0, position.width-1, position.height-1, colors.body );
		workArea->drawTrans(position.x,position.y,work);
		
		workArea->fill(Bitmap::makeColor(255,0,255));
		
		Bitmap::transBlender( 0, 0, 0, colors.borderAlpha );
		roundRect( workArea, position.radius, 0, 0, position.width-1, position.height-1, colors.border );
		workArea->drawTrans(position.x,position.y,work);
	} else {
		Bitmap::transBlender( 0, 0, 0, colors.bodyAlpha );
		workArea->rectangleFill( 0, 0, position.width-1, position.height-1, colors.body );
		workArea->drawTrans(position.x,position.y,work);
		
		workArea->fill(Bitmap::makeColor(255,0,255));
		
		Bitmap::transBlender( 0, 0, 0, colors.borderAlpha );
		workArea->rectangle( 0, 0, position.width-1, position.height-1, colors.border );
		workArea->drawTrans(position.x,position.y,work);
	}

	work.drawingMode( Bitmap::MODE_SOLID );
	
	workArea->fill(Bitmap::makeColor(255,0,255));
	
	if (currentSetFont){
		currentSetFont->printf(textX,textY,textColor,*workArea,currentSetText,0);
	}

	if(focused){
		if(cursorTime.msecs()<=blinkRate){
			workArea->line(cursorX,cursorY,cursorX,cursorY+textSizeH-5,textColor);
		}
	}
	
	workArea->draw(position.x,position.y,work);
}

// Keypresses
sigslot::slot LineEdit::keyPress(const keys &k)
{
	debug( 5 ) << "Received key press " << k.getValue() << std::endl;
	if(focused)
	{
		if(k.isCharacter())
		{
			char keyValue = k.getValue();
			bool addValue = false;
			
			switch(inputTypeValue){
			  case inputNumerical:
				if ( k.isNumber() ) addValue = !addValue;
				break;
			  case inputNoCaps:
				keyValue = tolower(keyValue);
				addValue = !addValue;
				break;
			  case inputAllCaps:
				keyValue = toupper(keyValue);
				addValue = !addValue;
				break;
			  case inputGeneral:
			  default:
				addValue = !addValue;
				break;
			}
			if(addValue){
			  if(limit!=0)
			  {
				  if(currentSetText.length()<limit)
				  {
					  //currentSetText += k.getValue();
					  currentSetText.insert(cursorIndex, std::string(1,keyValue));
					  ++cursorIndex;
					  changed();
				  }
			  }
			  else
			  {
				  //currentSetText += k.getValue();
				  currentSetText.insert(cursorIndex, std::string(1,keyValue));
				  ++cursorIndex;
				  changed();
			  }
			}
		}
		else
		{
			switch(k.getValue())
			{
				case keys::DEL:
					if(cursorIndex<currentSetText.length())
					{
						currentSetText.erase(cursorIndex,1);
					}
					break;
				case keys::BACKSPACE:
					if(cursorIndex>0)
					{
						currentSetText.erase(cursorIndex - 1, 1);
						--cursorIndex;
					}
					break;
				case keys::RIGHT:
					if(cursorIndex<currentSetText.length())++cursorIndex;
					break;
				case keys::LEFT:
					if(cursorIndex>0)--cursorIndex;
					break;
				case keys::INSERT:
					break;
			}
			changed();
		}
	}
}

// Set text
void LineEdit::setText(const std::string & text)
{
	currentSetText = text;
	if(limit!=0)
	{
		if(currentSetText.length()>limit)
		{
			while(currentSetText.length()>limit)
			{
				currentSetText.erase(currentSetText.begin()+currentSetText.length()-1);
			}
		}
	}
	cursorIndex = currentSetText.length();
	changed();
}


//! Get text
const std::string & LineEdit::getText()
{
	return currentSetText;
}
		
//! Clear text
void LineEdit::clearText()
{
	currentSetText.clear();
	cursorIndex=0;
	changed();
}

//! Set text limit
void LineEdit::setLimit(unsigned int l)
{
	limit = l;
	if(limit!=0)
	{
		if(currentSetText.length()>limit)
		{
			while(currentSetText.length()>limit)
			{
				currentSetText.erase(currentSetText.begin()+currentSetText.length()-1);
			}
		}
	}
	cursorIndex = currentSetText.length();
	changed();
}
		
// Set Horizontal Alignment
void LineEdit::setHorizontalAlign(const textAlign a)
{
	hAlignment = hAlignMod = a;
	changed();
}
		
// Set Vertical Alignment
void LineEdit::setVerticalAlign(const textAlign a)
{
	vAlignment = a;
	changed();
}

//! Set the type of input default general
void LineEdit::setInputType(const inputType i){
	inputTypeValue = i;
}
		
// Set textColor
void LineEdit::setTextColor(const int color)
{
	textColor = color;
}


//! Set textColor
void LineEdit::setCursorColor(const int color)
{
	textColor = color;
}

// Set font
void LineEdit::setFont(const Font *f)
{
	currentSetFont = f;
	if(currentSetFont) changed();
}

// Set autoResizeable
void LineEdit::setAutoResize(bool r)
{
	autoResizable = r;
}

// Set the cursor blink rate in miliseconds (default 500)
void LineEdit::setCursorBlinkRate(unsigned int msecs)
{
	blinkRate = msecs;
}

//! set Focus
void LineEdit::setFocused(bool focus)
{
	focused = focus;
}

//! check Focus
bool LineEdit::isFocused()
{ return focused; }

