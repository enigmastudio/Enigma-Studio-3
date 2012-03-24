
#include "hwsynth.hpp"

const eU32 DISPLAY_WIDTH = 480;
const eU32 DISPLAY_HEIGHT = 272;

const eU32 DISPLAY_WIDTH_D2 = DISPLAY_WIDTH / 2;
const eU32 DISPLAY_WIDTH_D4 = DISPLAY_WIDTH / 4;
const eU32 DISPLAY_WIDTH_D8 = DISPLAY_WIDTH / 8;

const eU32 DISPLAY_HEIGHT_D2 = DISPLAY_HEIGHT / 2;
const eU32 DISPLAY_HEIGHT_D4 = DISPLAY_HEIGHT / 4;

const eU32 DISPLAY_HEIGHT_1Q = DISPLAY_HEIGHT / 4;
const eU32 DISPLAY_HEIGHT_3Q = DISPLAY_HEIGHT_1Q * 3;

tfDisplay::tfDisplay() :
	m_surface(eNULL),
	m_changed(eTRUE),
	m_displayValue(0.0f)
{

}

tfDisplay::~tfDisplay()
{
	closeDisplay();
}

eBool tfDisplay::openDisplay()
{
	printf ("opening display.\n");

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) 
	{
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        	return eFALSE;
    	}

	if (TTF_Init() < 0)
	{
	    	printf("Couldn't initialize SDL TTF: %s\n", SDL_GetError());
	    	return eFALSE;
	}

    	if ((m_surface = SDL_SetVideoMode(DISPLAY_WIDTH, DISPLAY_HEIGHT, 32, SDL_HWSURFACE | SDL_DOUBLEBUF)) == eNULL) 
	{
		printf("Couldn't set video mode: %s\n", SDL_GetError());
        	return eFALSE;
    	}

	SDL_WM_SetCaption("Tunefish3 - HW", "Tunefish3");

	m_fontBig = _loadFont("FreeSans.ttf", 24);
	m_fontMed = _loadFont("FreeSans.ttf", 14);
	m_fontSmall = _loadFont("FreeSans.ttf", 10);

	return eTRUE;
}

void tfDisplay::closeDisplay()
{
	if (m_surface == eNULL)
		return;

	_freeFont(m_fontBig);
	_freeFont(m_fontMed);
	_freeFont(m_fontSmall);

	SDL_FreeSurface(m_surface);
	m_surface = eNULL;

    	SDL_Quit();
	TTF_Quit();

	printf ("display freed.\n");
}

eBool tfDisplay::process()
{
	SDL_Event event;

    	while(SDL_PollEvent(&event)) 
	{
		if(event.type == SDL_QUIT) 
			return eFALSE;
	}

	if (m_changed)
	{
		SDL_FillRect(m_surface, NULL, 0);

		SDL_Color fgColor={255,255,255,255};
		SDL_Color bgColor={0,0,0,255};

		// render scopes
		eS8 *scope1 = m_scopes[0];
		eS8 *scope2 = m_scopes[1];
		eS8 last1 = *scope1;
		eS8 last2 = *scope2;

		for(eU32 i=0;i<SCOPESIZE;i++)
		{
			lineColor(m_surface, 90 + i, DISPLAY_HEIGHT_3Q + last1, 91 + i, DISPLAY_HEIGHT_3Q + *scope1, 0xffffffff);
			lineColor(m_surface, 166 + i, DISPLAY_HEIGHT_3Q + last2, 167 + i, DISPLAY_HEIGHT_3Q + *scope2, 0xffffffff);
			last1 = *scope1++;
			last2 = *scope2++;
		}

		// center text
		_drawFloat(m_displayValue, DISPLAY_WIDTH_D2, DISPLAY_HEIGHT_D2 + 10, m_fontBig, ORNT_CENTER, ORNT_CENTER, fgColor, bgColor);
		_drawText(m_displayString, DISPLAY_WIDTH_D2, DISPLAY_HEIGHT_D2 - 15, m_fontMed, ORNT_CENTER, ORNT_CENTER, fgColor, bgColor);

		// left buttons
		_drawText("Button 1", 4, 4, m_fontSmall, ORNT_LEFT, ORNT_LEFT, fgColor, bgColor);	
		_drawText("Button 2", 4, DISPLAY_HEIGHT_D4 + 4, m_fontSmall, ORNT_LEFT, ORNT_LEFT, fgColor, bgColor);	
		_drawText("Button 3", 4, DISPLAY_HEIGHT_D4 * 2 + 4, m_fontSmall, ORNT_LEFT, ORNT_LEFT, fgColor, bgColor);	
		_drawText("Button 4", 4, DISPLAY_HEIGHT_D4 * 3 + 4, m_fontSmall, ORNT_LEFT, ORNT_LEFT, fgColor, bgColor);

		// right buttons
		_drawText("Button 5", DISPLAY_WIDTH - 4, 4, m_fontSmall, ORNT_RIGHT, ORNT_LEFT, fgColor, bgColor);	
		_drawText("Button 6", DISPLAY_WIDTH - 4, DISPLAY_HEIGHT_D4 + 4, m_fontSmall, ORNT_RIGHT, ORNT_LEFT, fgColor, bgColor);	
		_drawText("Button 7", DISPLAY_WIDTH - 4, DISPLAY_HEIGHT_D4 * 2 + 4, m_fontSmall, ORNT_RIGHT, ORNT_LEFT, fgColor, bgColor);	
		_drawText("Button 8", DISPLAY_WIDTH - 4, DISPLAY_HEIGHT_D4 * 3 + 4, m_fontSmall, ORNT_RIGHT, ORNT_LEFT, fgColor, bgColor);	

		// botton knobs
		_drawText("Knob 1", DISPLAY_WIDTH_D8, DISPLAY_HEIGHT - 10, m_fontSmall, ORNT_CENTER, ORNT_CENTER, fgColor, bgColor);	
		_drawText("Knob 2", DISPLAY_WIDTH_D4 + DISPLAY_WIDTH_D8, DISPLAY_HEIGHT - 10, m_fontSmall, ORNT_CENTER, ORNT_CENTER, fgColor, bgColor);	
		_drawText("Knob 3", DISPLAY_WIDTH_D4 * 2 + DISPLAY_WIDTH_D8, DISPLAY_HEIGHT - 10, m_fontSmall, ORNT_CENTER, ORNT_CENTER, fgColor, bgColor);	
		_drawText("Knob 4", DISPLAY_WIDTH_D4 * 3 + DISPLAY_WIDTH_D8, DISPLAY_HEIGHT - 10, m_fontSmall, ORNT_CENTER, ORNT_CENTER, fgColor, bgColor);	

		m_changed = eFALSE;
		SDL_Flip(m_surface);
	}

	return eTRUE;
}

void tfDisplay::setScopes(eS16 *output, eU32 len)
{
	len = len > SCOPESIZE ? SCOPESIZE : len;
	eS8 *scope1 = m_scopes[0];
	eS8 *scope2 = m_scopes[1];

	while(len--)
	{
		*scope1++ = *output++ / (256 * 4);
		*scope2++ = *output++ / (256 * 4);
	}

	m_changed = eTRUE;
}

void tfDisplay::setDisplayString(string str)
{
	m_changed = eTRUE;
	m_displayString = str;
}

void tfDisplay::setDisplayValue(eF32 value)
{
	m_changed = eTRUE;
	m_displayValue = value;
}

tfDisplay::Font * tfDisplay::_loadFont(const eChar *name, eU32 size)
{
	Font *font = new Font();
	printf("loading font: %s, %i\n", name, size);

	font->ttf = TTF_OpenFont(name, size);
	if(!font->ttf)
	{
		printf("TTF_OpenFont: %s\n", TTF_GetError());
		delete font;
		return eNULL;
	}

	SDL_Color fg={0,0,0,255};

	font->height = TTF_FontLineSkip(font->ttf);

	for (eU32 i=0; i<128; i++)
	{
		font->surfaces[i] = TTF_RenderGlyph_Solid(font->ttf, i, fg);

		if(!font->surfaces[i])
			printf("TTF_RenderGlyph_Solid: %s\n", TTF_GetError());

		TTF_GlyphMetrics(font->ttf, i,
				&font->gm[i].minx, &font->gm[i].maxx,
				&font->gm[i].miny, &font->gm[i].maxy,
				&font->gm[i].advance);
	}

	return font;
}

void tfDisplay::_freeFont(tfDisplay::Font* font)
{
	if (font == eNULL)
		return;

	TTF_CloseFont(font->ttf);

	for(eU32 i=0; i<128; i++)
	{
		if(font->surfaces[i]) 
			SDL_FreeSurface(font->surfaces[i]);
	}

	delete font;
}

void tfDisplay::_drawFloat(eF32 value, eU32 x, eU32 y, 
			  tfDisplay::Font *font, Orientation orntX, Orientation orntY, 
			  SDL_Color fgColor, SDL_Color bgColor)
{
	eChar str[32];
	sprintf(str, "%.02f", value);
	string text = str;
	_drawText(text, x, y, font, orntX, orntY, fgColor, bgColor);
}

void tfDisplay::_drawText(const string text, eU32 x, eU32 y, 
			  tfDisplay::Font *font, Orientation orntX, Orientation orntY, 
			  SDL_Color fgColor, SDL_Color bgColor)
{
	SDL_Surface *surface = TTF_RenderUTF8_Shaded(font->ttf, text.c_str(), fgColor, bgColor);

	if (surface == NULL)
	{
		printf("Couldn't create String: %s\n", SDL_GetError());
		return;
	}
    
	SDL_Rect dest;
	dest.w = surface->w;
	dest.h = surface->h;

	switch (orntX)
	{
		case ORNT_LEFT: 	dest.x = x; break;
		case ORNT_CENTER:	dest.x = x - (surface->w / 2); break;
		case ORNT_RIGHT:	dest.x = x - surface->w; break;
	}

	switch (orntY)
	{
		case ORNT_LEFT: 	dest.y = y; break;
		case ORNT_CENTER:	dest.y = y - (surface->h / 2); break;
		case ORNT_RIGHT:	dest.y = y - surface->h; break;
	}

	SDL_BlitSurface(surface, NULL, m_surface, &dest);
	SDL_FreeSurface(surface);
}





