
#ifndef TF_DISPLAY_HPP
#define TF_DISPLAY_HPP

const eU32 SCOPESIZE = 64;

class tfDisplay
{
public:
	enum Orientation
	{
		ORNT_LEFT,
		ORNT_CENTER,
		ORNT_RIGHT
	};

	struct GlyphMetrics
	{
		eS32 minx;
		eS32 maxx;
		eS32 miny;
		eS32 maxy;
		eS32 advance;
	};

	struct Font
	{
		TTF_Font *ttf;
		SDL_Surface *surfaces[128];
		GlyphMetrics gm[128];
		eU32 height;
	};

	tfDisplay();
	~tfDisplay();

	eBool 	openDisplay();
	void 	closeDisplay();
	eBool 	process();

	void	setScopes(eS16 *output, eU32 len);
	void	setDisplayString(string str);
	void	setDisplayValue(eF32 value);

private:
	Font *	_loadFont(const eChar *name, eU32 size);
	void	_freeFont(Font* font);
	void	_drawFloat(eF32 value, eU32 x, eU32 y, 
			  tfDisplay::Font *font, Orientation orntX, Orientation orntY, 
			  SDL_Color fgColor, SDL_Color bgColor);
	void	_drawText(const string text, eU32 x, eU32 y, 
			  tfDisplay::Font *font, Orientation orntX, Orientation orntY, 
			  SDL_Color fgColor, SDL_Color bgColor);

	SDL_Surface* 		m_surface;
	eBool			m_changed;
	string 			m_displayString;
	eF32			m_displayValue;
	Font *			m_fontBig;
	Font *			m_fontMed;
	Font *			m_fontSmall;
	eS8 			m_scopes[2][SCOPESIZE];
};

#endif
