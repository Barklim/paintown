#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <vector>

#include "mugen_font.h"

#include "mugen_item.h"
#include "mugen_item_content.h"
#include "mugen_section.h"
#include "mugen_reader.h"
#include "mugen_util.h"
#include "globals.h"
#include "util/funcs.h"

// If you use this, please delete the item after you use it, this isn't java ok
static MugenItemContent *getOpts( const std::string &opt ){
    std::string contentHolder = "";
    MugenItemContent *temp = new MugenItemContent();
    const char * ignored = " \r\n";
    Global::debug(1) << "Parsing string to ItemContent: " << opt << endl;
    for( unsigned int i = 0; i < opt.size(); ++i ){
        if( opt[i] == ';' )break;
        if( opt[i] == ' ' ){
            if( !contentHolder.empty() ) *temp << contentHolder;
            Global::debug(1) << "Got content: " << contentHolder << endl;
            contentHolder = "";
        }
        //Start grabbing our item
        else if (! strchr(ignored, opt[i])){
            contentHolder += opt[i];
        }
    }
    if( !contentHolder.empty() ){
        *temp << contentHolder;
        Global::debug(1) << "Got content: " << contentHolder << endl;
    }
    return temp;
}


MugenFont::MugenFont( const string & file ):
type(Fixed),
width(0),
height(0),
spacingx(0),
spacingy(0),
colors(0),
offsetx(0),
offsety(0),
bmp(0),
pcx(0),
pcxsize(0),
currentBank(0){
    std::string temp = file;
    MugenUtil::invertSlashes(temp);
    Global::debug(1) << "[mugen font] Opening file '" << temp << "'" << endl;
    ifile.open( temp.c_str() );
    if (!ifile){
        perror("cant open file");
    }
    myfile = temp;
    
    load();
}

MugenFont::MugenFont( const char * file ):
type(Fixed),
width(0),
height(0),
spacingx(0),
spacingy(0),
colors(0),
offsetx(0),
offsety(0),
bmp(0),
pcx(0),
pcxsize(0),
currentBank(0){
    std::string temp = file;
    MugenUtil::invertSlashes(temp);
    Global::debug(1) << "[mugen font] Opening file '" << temp << "'" << endl;
    ifile.open( temp.c_str() );
    if (!ifile){
        perror("cant open file");
    }
    myfile = temp;
    
    load();
}

MugenFont::MugenFont( const MugenFont &copy ){
    this->type = copy.type;
    this->width = copy.width;
    this->height = copy.height;
    this->spacingx = copy.spacingx;
    this->spacingy = copy.spacingy;
    this->colors = copy.colors;
    this->offsetx = copy.offsetx;
    this->offsety = copy.offsety;
    this->bmp = copy.bmp;
}

MugenFont::~MugenFont(){
    if(bmp){
        delete bmp;
    }
}
    
MugenFont & MugenFont::operator=( const MugenFont &copy ){
    this->type = copy.type;
    this->width = copy.width;
    this->height = copy.height;
    this->spacingx = copy.spacingx;
    this->spacingy = copy.spacingy;
    this->colors = copy.colors;
    this->offsetx = copy.offsetx;
    this->offsety = copy.offsety;
    this->bmp = copy.bmp;
    
    return *this;
}

// Implement Font stuff
void MugenFont::setSize( const int x, const int y ){
    // We don't change sizes
    if  ( (x < 0 || y < 0) && type!=Fixed ){
        return;
    }
}
const int MugenFont::getSizeX() const{
    return width;
}
const int MugenFont::getSizeY() const{
    return height;
}

const int MugenFont::textLength( const char * text ) const{
    std::string str(text);
    int size =0;
    for (unsigned int i = 0; i < str.size(); ++i){
        std::map<char, FontLocation>::const_iterator loc = positions.find(str[i]);
        if (loc!=positions.end()){
            size+=loc->second.width + spacingx;
        } else{
            // Couldn't find a position for this character assume regular width and skip to the next character
            size+=width + spacingx;
        }
    }
    return size;
}

const int MugenFont::getHeight( const string & str ) const{
    // What? I guess this is for freetype?
    return getHeight();
}

const int MugenFont::getHeight() const{
    return height;
}

void MugenFont::printf( int x, int y, int color, const Bitmap & work, const string & str, int marker, ... ) const{
    // Va list
    char buf[512];
    va_list ap;

    va_start(ap, marker);
    vsnprintf(buf, sizeof(buf), str.c_str(), ap);
    va_end(ap);

    const std::string newstr(buf);

    int workoffsetx = 0;
    for (unsigned int i = 0; i < newstr.size(); ++i){
        std::map<char, FontLocation>::const_iterator loc = positions.find(newstr[i]);
        if (loc!=positions.end()){
            Bitmap character = Bitmap::temporaryBitmap( loc->second.width + spacingx, height + spacingy);
            bmp->Blit( loc->second.startx, 0, loc->second.width + spacingx, height + spacingy,0,0, character );
            character.draw( x + workoffsetx, y, work);
            workoffsetx+=loc->second.width + spacingx;
        } else{
            // Couldn't find a position for this character draw nothing, assume width, and skip to the next character
            workoffsetx+=width + spacingx;
        }
    }
}

/* get a pointer to a specific bank. bank numbers start from 0 */
unsigned char * MugenFont::findBank(int bank){
    return pcx + pcxsize - ((bank+1) * colors * 3);
}

void MugenFont::changeBank(int bank){
    if (bank < 0 || bank > (colors -1))return;
    currentBank = bank;
    unsigned char newpal[768];
    // Reset palette
    memcpy(pcx + (pcxsize - 768), palette, 768);
    memcpy( newpal, pcx+(pcxsize)-768, 768);
    unsigned char * newBank = findBank(bank);
    Global::debug(1) << "palette start: " << (void*)(pcx + pcxsize - 768) << ". new bank: " << (void*)newBank << endl;
    memcpy((void*) &newpal[768 - colors*3], (void*) newBank, colors * 3);

    /* hack to get around the 16-bit masking color */
    for (int i = 768 - colors * 3; i < 768; i += 3){
        int r = newpal[i];
        int g = newpal[i+1];
        int b = newpal[i+2];
        int col = Bitmap::makeColor(r,g,b);
        if (col == Bitmap::MaskColor){
            int oldCol = col;
            while (oldCol == col){
                r -= 1;
                oldCol = Bitmap::makeColor(r,g,b);
            }
            newpal[i] = r;
            newpal[i+1] = g;
            newpal[i+2] = b;
        }
    }

    memcpy(pcx + (pcxsize - 768), newpal, 768);
    if (bmp){
        delete bmp;
    }
    bmp = new Bitmap(Bitmap::memoryPCX((unsigned char*) pcx, pcxsize));
}

void MugenFont::load(){

    /* 16 skips the header stuff */
    int location = 16;
    // Lets go ahead and skip the crap -> (Elecbyte signature and version) start at the 16th byte
    ifile.seekg(location,ios::beg);
    unsigned long pcxlocation;
    unsigned long txtlocation;
    unsigned long txtsize;

    ifile.read( (char *)&pcxlocation, sizeof(unsigned long) );
    ifile.read( (char *)&pcxsize, sizeof(unsigned long) );
    ifile.read( (char *)&txtlocation, sizeof(unsigned long) );
    ifile.read( (char *)&txtsize, sizeof(unsigned long) );
    Global::debug(1) << "PCX Location: " << pcxlocation << " | PCX Size: " << pcxsize << endl;
    Global::debug(1) << "TXT Location: " << txtlocation << " | TXT Actual location: " << pcxlocation + pcxsize << " | TXT Size: " << txtsize << endl;

    // Get the pcx load our bitmap
    ifile.seekg(pcxlocation,ios::beg);
    pcx = new unsigned char[pcxsize];
    ifile.read((char *)pcx, pcxsize);
    memcpy( palette, pcx+(pcxsize)-768, 768);

    bmp = new Bitmap(Bitmap::memoryPCX((unsigned char*) pcx, pcxsize));

    // Get the text
    ifile.seekg(pcxlocation+pcxsize, ios::beg);
    std::vector<std::string> ourText;
    while( !ifile.eof() ){
        std::string line;
        getline( ifile, line );
        ourText.push_back(line);
    }

    std::vector< MugenSection * > collection = MugenUtil::configReader( ourText );

    for( unsigned int i = 0; i < collection.size(); ++i ){
        const std::string &head = collection[i]->getHeader();
        if( head == "Def" ){
            while( collection[i]->hasItems() ){
                MugenItemContent *content = collection[i]->getNext();
                const MugenItem *item = content->getNext();
                std::string itemhead = item->query();
                MugenUtil::removeSpaces(itemhead);
                // This is so we don't have any problems with crap like Name, NaMe, naMe or whatever
                MugenUtil::fixCase( itemhead );
                if ( itemhead.find("size")!=std::string::npos ){
                    *content->getNext() >> width;
                    *content->getNext() >> height;
                } else if ( itemhead.find("spacing")!=std::string::npos ){
                    *content->getNext() >> spacingx;
                    *content->getNext() >> spacingy;
                } else if ( itemhead.find("colors")!=std::string::npos ){
                    *content->getNext() >> colors;
                } else if ( itemhead.find("offset")!=std::string::npos ){
                    *content->getNext() >> offsetx;
                    *content->getNext() >> offsety;
                } else if ( itemhead.find("type")!=std::string::npos ){
                    std::string temp;
                    *content->getNext() >> temp;
                    MugenUtil::removeSpaces(temp);
                    MugenUtil::fixCase(temp);
                    if (temp == "variable")type = Variable;
                    else if (temp == "fixed")type = Fixed;
                    Global::debug(1) << "Type: " << temp << endl;
                } //else throw MugenException( "Unhandled option in Info Section: " + itemhead );
            }
            Global::debug(1) << "Size X: " << width << ", Size Y: " << height << ", Spacing X: " << spacingx << ", Spacing Y: " << spacingy << ", Colors: " << colors << ", Offset X: " << offsetx << ", Offset Y: " << offsety << endl;
        }
        if( head == "Map" ){
            bool beginParse = false;
            int locationx = 0;
            for (std::vector<std::string>::iterator l = ourText.begin(); l != ourText.end(); ++l){
                std::string line = *l;
                if (!beginParse){
                    if (line.find("[Map]")==std::string::npos){
                        continue;
                    } else{
                        beginParse = true;
                        continue;
                    }
                }
                MugenItemContent *opt = getOpts(line);
                std::string character;
                int startx = locationx * width;
                int chrwidth = width;
		if (opt->hasItems()){
		    *opt->getNext() >> character;
		    if (character.size() > 1){
			MugenUtil::fixCase(character);
			// 0x5b
			if (character.compare("0x5b")==0)character = "[";
			// 0x3b
			else if (character.compare("0x3b")==0)character = ";";
		    }
                    if (type != Fixed){
                        // get other two
                        *opt->getNext() >> startx;
                        *opt->getNext() >> chrwidth;
                    }
                    FontLocation loc;
                    loc.startx = startx;
                    loc.width = chrwidth;
                    char code = character[0];
                    Global::debug(1) << "Storing Character: " << code << " | startx: " << loc.startx << " | width: " << loc.width << endl;
                    positions[code] = loc;
                }
                delete opt;
                ++locationx;
            }
        }
    }

    // Time to get rid of collection
    for( std::vector< MugenSection * >::iterator i = collection.begin() ; i != collection.end() ; ++i ){
        if(*i)delete (*i);
    }

    ifile.close();

}
