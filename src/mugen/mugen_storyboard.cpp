#include "mugen_storyboard.h"

#include "util/bitmap.h"
#include "init.h"
#include "resource.h"
#include "util/funcs.h"
#include "game/console.h"
#include "object/animation.h"
#include "object/object.h"
#include "object/character.h"
#include "object/object_attack.h"
#include "object/player.h"
#include "globals.h"
#include "factory/font_render.h"

#include "mugen_animation.h"
#include "mugen_background.h"
#include "mugen_item.h"
#include "mugen_item_content.h"
#include "mugen_section.h"
#include "mugen_sound.h"
#include "mugen_reader.h"
#include "mugen_sprite.h"
#include "mugen_util.h"
#include "mugen_font.h"

const int DEFAULT_WIDTH = 320;
const int DEFAULT_HEIGHT = 240;
const int DEFAULT_SCREEN_X_AXIS = 160;
const int DEFAULT_SCREEN_Y_AXIS = 0;

MugenScene::MugenScene():
clearColor(Bitmap::makeColor(0,0,0)),
ticker(0),
endTime(0),
backgroundName(""){
}
MugenScene::~MugenScene(){
}
void MugenScene::act(){
}
void MugenScene::draw(Bitmap *bmp){
}

MugenStoryboard::MugenStoryboard( const string & s ):
location(s),
spriteFile(""),
startscene(0){
}
MugenStoryboard::~MugenStoryboard(){
    cleanup();
}
void MugenStoryboard::load() throw (MugenException){
     // Lets look for our def since some assholes think that all file systems are case insensitive
    std::string baseDir = Util::getDataPath() + "mugen/data/" + MugenUtil::getFileDir(location);
    const std::string ourDefFile = MugenUtil::fixFileName( baseDir, MugenUtil::stripDir(location) );
    // get real basedir
    //baseDir = MugenUtil::getFileDir( ourDefFile );
    Global::debug(1) << baseDir << endl;
    
    if( ourDefFile.empty() )throw MugenException( "Cannot locate storyboard definition file for: " + location );
    
    std::string filesdir = "";
    
    Global::debug(1) << "Got subdir: " << filesdir << endl;
     
    MugenReader reader( ourDefFile );
    std::vector< MugenSection * > collection;
    collection = reader.getCollection();
    
    // for linked position in backgrounds
    MugenBackground *prior = 0;
    
    /* Extract info for our first section of our stage */
    for( unsigned int i = 0; i < collection.size(); ++i ){
	std::string head = collection[i]->getHeader();
	MugenUtil::fixCase(head);
	if( head == "info" ){
	    while( collection[i]->hasItems() ){
		MugenItemContent *content = collection[i]->getNext();
		const MugenItem *item = content->getNext();
		std::string itemhead = item->query();
		MugenUtil::removeSpaces(itemhead);
		if ( itemhead.find("name")!=std::string::npos ){
		    std::string temp;
		    *content->getNext() >> temp;
		    Global::debug(1) << "Read name '" << temp << "'" << endl;
		} else if ( itemhead.find("author")!=std::string::npos ){
		    std::string temp;
		    *content->getNext() >> temp;
                    Global::debug(1) << "Made by: '" << temp << "'" << endl;
		} else throw MugenException( "Unhandled option in Info Section: " + itemhead );
	    }
	}
	else if( head == "scenedef" ){
	    while( collection[i]->hasItems() ){
		MugenItemContent *content = collection[i]->getNext();
		const MugenItem *item = content->getNext();
		std::string itemhead = item->query();
		MugenUtil::removeSpaces(itemhead);
		MugenUtil::fixCase(itemhead);
		if ( itemhead.find("spr")!=std::string::npos ){
		    *content->getNext() >> spriteFile;
		    Global::debug(1) << "Got Sprite File: '" << spriteFile << "'" << endl;
		    MugenUtil::readSprites( MugenUtil::getCorrectFileLocation(baseDir, spriteFile), "", sprites );
		} else if ( itemhead.find("startscene")!=std::string::npos ){
		    *content->getNext() >> startscene;
                    Global::debug(1) << "Starting storyboard at: '" << startscene << "'" << endl;
		} else throw MugenException( "Unhandled option in Files Section: " + itemhead );
	    }
	}
	else if( head == "scene " ){
	    MugenScene *scene = new MugenScene();
	    while( collection[i]->hasItems() ){
		MugenItemContent *content = collection[i]->getNext();
		const MugenItem *item = content->getNext();
		std::string itemhead = item->query();
		MugenUtil::removeSpaces(itemhead);
		MugenUtil::fixCase(itemhead);
		if ( itemhead.find("fadein.time")!=std::string::npos ){
		    int time;
		    *content->getNext() >> time;
		    scene->fader.setFadeInTime(time);
		} else if ( itemhead.find("fadein.color")!=std::string::npos ){
		    int r,g,b;
		    *content->getNext() >> r;
		    *content->getNext() >> g;
		    *content->getNext() >> b;
		    scene->fader.setFadeInColor(Bitmap::makeColor(r,g,b));
		} else if ( itemhead.find("fadeout.time")!=std::string::npos ){
		    int time;
		    *content->getNext() >> time;
		    scene->fader.setFadeOutTime(time);
		} else if ( itemhead.find("fadeout.color")!=std::string::npos ){
		    int r,g,b;
		    *content->getNext() >> r;
		    *content->getNext() >> g;
		    *content->getNext() >> b;
		    scene->fader.setFadeOutColor(Bitmap::makeColor(r,g,b));
		} else if ( itemhead.find("bg.name")!=std::string::npos ){
		    *content->getNext() >> scene->backgroundName;
		} else if ( itemhead.find("clearcolor")!=std::string::npos ){
		    int r,g,b;
		    *content->getNext() >> r;
		    *content->getNext() >> g;
		    *content->getNext() >> b;
		    scene->clearColor = Bitmap::makeColor(r,g,b);
		} else if ( itemhead.find("end.time")!=std::string::npos ){
		    *content->getNext() >> scene->endTime;
		} else throw MugenException( "Unhandled option in Scene Section: " + itemhead );
	    }
	    scenes.push_back(scene);
	}
	else if( head.find("begin action") != std::string::npos ){
	    head.replace(0,13,"");
	    int h;
	    MugenItem(head) >> h;
	    animations[h] = MugenUtil::getAnimation(collection[i], sprites);
	}
	else throw MugenException( "Unhandled Section in '" + ourDefFile + "': " + head ); 
    }
    // Set up the animations for those that have action numbers assigned (not -1 )
    // Also do their preload
    for( std::vector< MugenScene * >::iterator s = scenes.begin(); s != scenes.end(); ++s ){
	MugenScene *scene = *s;
	for( std::vector< MugenBackground * >::iterator i = scene->backgrounds.begin(); i != scene->backgrounds.end(); ++i ){
	    MugenBackground *background = *i;
	    if( background->getActionNumber() != -1 ){
		background->setAnimation( animations[ background->getActionNumber() ] );
	    }
	    // now load
	    (*i)->preload( DEFAULT_SCREEN_X_AXIS, DEFAULT_SCREEN_Y_AXIS );
	}
    }
}
void MugenStoryboard::run(Bitmap *bmp){
}
void MugenStoryboard::cleanup(){
    
}


