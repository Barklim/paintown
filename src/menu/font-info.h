#ifndef _paintown_font_info_h
#define _paintown_font_info_h

#include "util/file-system.h"

class Font;

namespace Menu{

/*
 * class FontInfo
 * class DefaultFontInfo
 * class RelativeFontInfo
 * class AbsoluteFontInfo
 */

class FontInfo {
    public:
	FontInfo();
	FontInfo(const Filesystem::RelativePath & font, int width, int height);
	FontInfo(const FontInfo &);
	virtual ~FontInfo();
	
	FontInfo & operator=(const FontInfo &);
	
	inline void set(const Filesystem::RelativePath & font, int width, int height){
	    this->font = font;
	    this->width = width;
	    this->height = height;
	}
	
	virtual const Font & get() const;
	virtual const Font & get(const Font & next) const;
        virtual const FontInfo & get(const FontInfo & next) const;
	
	inline void setFont(const Filesystem::RelativePath & font){
	    this->font = font;
	}

	virtual const Filesystem::RelativePath getFont(const FontInfo & next) const;
	
	inline void setWidth(int width){
	    this->width = width;
	}

	virtual const int getWidth(const FontInfo & next) const;
	
	inline void setHeight(int height){
	    this->height = height;
	}

	virtual const int getHeight(const FontInfo & next) const;
	
	inline const bool empty() const{
	    return font.path().empty();
	}
	
    private:
	Filesystem::RelativePath font;
	int width;
	int height;
};

class DefaultFontInfo: public FontInfo {
public:
    virtual const Font & get() const;
    virtual const Font & get(const Font & next) const;
    virtual const FontInfo & get(const FontInfo & next) const;
    virtual const Filesystem::RelativePath getFont(const FontInfo & next) const;
};

}

#endif