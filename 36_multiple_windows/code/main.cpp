//Using SDL, SDL_image, SDL_ttf, standard IO, strings, and string streams
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <sstream>


//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//The dimensions of the level
const int LEVEL_WIDTH = SCREEN_WIDTH;
const int LEVEL_HEIGHT = SCREEN_HEIGHT;

const int TOTAL_DATA = 10;

//Total windows
const int TOTAL_WINDOWS = 3; 

//A circle structure
struct Circle
{
	int x, y;
	int r;
};

//Texture wrapper class
class LTexture
{
	public:
		//Initializes variables
		LTexture();

		//Deallocates memory
		~LTexture();

		//Loads image at specified path
		bool loadFromFile( std::string path );
		
		#ifdef _SDL_TTF_H
		//Creates image from font string
		bool loadFromRenderedText( std::string textureText, SDL_Color textColor );
		#endif

		//Deallocates texture
		void free();

		//Set color modulation
		void setColor( Uint8 red, Uint8 green, Uint8 blue );

		//Set blending
		void setBlendMode( SDL_BlendMode blending );

		//Set alpha modulation
		void setAlpha( Uint8 alpha );
		
		//Renders texture at given point
		void render( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE );

		//Gets image dimensions
		int getWidth();
		int getHeight();

	private:
		//The actual hardware texture
		SDL_Texture* mTexture;

		//Image dimensions
		int mWidth;
		int mHeight;
};

//The application time based timer
class LTimer
{
    public:
		//Initializes variables
		LTimer();

		//The various clock actions
		void start();
		void stop();
		void pause();
		void unpause();

		//Gets the timer's time
		Uint32 getTicks();

		//Checks the status of the timer
		bool isStarted();
		bool isPaused();

    private:
		//The clock time when the timer started
		Uint32 mStartTicks;

		//The ticks stored when the timer was paused
		Uint32 mPausedTicks;

		//The timer status
		bool mPaused;
		bool mStarted;
};

//The dot that will move around on the screen
class Dot
{
	public:
		//The dimensions of the dot
		static const int DOT_WIDTH = 20;
		static const int DOT_HEIGHT = 20;
		
		//Maximum axis velocity of the dot
		static const int DOT_VEL = 10;
		
		//Initializes the variables
		Dot();
		
		//Takes key presses and adjusts the dot's velocity
		void handleEvent(SDL_Event& e);
		
		//Moves the dot
		void move();
		
		//Shows the dot on the scren
		void render();
		
	private:
		//The X and Y offsets of the dot
		int mPosX, mPosY;
		
		//The velocity of the dot
		int mVelX, mVelY;
		
		//Dot's collision boxes
		Circle mCollider;
		
		//Moves the collision circle relative to the dot's offset
		void shiftColliders();
};

class LWindow
{
	public:
		//Initializes internals
		LWindow();
		
		//Creates window
		bool init();
			
		//Handles window events
		void handleEvent(SDL_Event& e);
		
		//Focuses on window
		void focus();
		
		//Shows windows contents
		void render();
		
		//Deallocates internals
		void free();
		
		//Window dimensions
		int getWidth();
		int getHeight();
		
		//Window focii
		bool hasMouseFocus();
		bool hasKeyboardFocus();
		bool isMinimized();
		bool isShown();
		
	private:
		//Window Data
		SDL_Window* mWindow;
		SDL_Renderer* mRenderer;
		int mWindowID;
		
		//Window dimensions
		int mWidth;
		int mHeight;
		
		//Window focus
		bool mMouseFocus;
		bool mKeyboardFocus;
		bool mFullScreen;
		bool mMinimized;	
		bool mShown;
};

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

//Circle/Circle collision detector
bool checkCollision(Circle& a, Circle& b);

//Circle/Box collision detector
bool checkCollision(Circle& a, SDL_Rect& b);

//Calculates distance squared between two points
double distanceSquared(int x1, int y1, int x2, int y2);

//Globally used font
TTF_Font* gFont = NULL;

//Scene textures
LTexture gInputTextTexture;
LTexture gPromptTextTexture;
LTexture gDataTextures[TOTAL_DATA];

//Data points
Sint32 gData[TOTAL_DATA];

//Our custom window
LWindow gWindows[TOTAL_WINDOWS];

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Scene textures
LTexture gSceneTexture;

LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	//Deallocate
	free();
}

bool LTexture::loadFromFile( std::string path )
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
	if( loadedSurface == NULL )
	{
		printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
	}
	else
	{
		//Color key image
		SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0, 0xFF, 0xFF ) );

		//Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
		if( newTexture == NULL )
		{
			printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface( loadedSurface );
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

#ifdef _SDL_TTF_H
bool LTexture::loadFromRenderedText( std::string textureText, SDL_Color textColor )
{
	//Get rid of preexisting texture
	free();
	
	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid( gFont, textureText.c_str(), textColor );
	if( textSurface != NULL )
	{
		//Create texture from surface pixels
        mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
		if( mTexture == NULL )
		{
			printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
		}
		else
		{
			//Get image dimensions
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}

		//Get rid of old surface
		SDL_FreeSurface( textSurface );
	}
	else
	{
		printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
	}
	
	
	//Return success
	return mTexture != NULL;
}
#endif

void LTexture::free()
{
	//Free texture if it exists
	if( mTexture != NULL )
	{
		SDL_DestroyTexture( mTexture );
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::setColor( Uint8 red, Uint8 green, Uint8 blue )
{
	//Modulate texture rgb
	SDL_SetTextureColorMod( mTexture, red, green, blue );
}

void LTexture::setBlendMode( SDL_BlendMode blending )
{
	//Set blending function
	SDL_SetTextureBlendMode( mTexture, blending );
}
		
void LTexture::setAlpha( Uint8 alpha )
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod( mTexture, alpha );
}

void LTexture::render( int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip )
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if( clip != NULL )
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopyEx( gRenderer, mTexture, clip, &renderQuad, angle, center, flip );
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}

LTimer::LTimer()
{
    //Initialize the variables
    mStartTicks = 0;
    mPausedTicks = 0;

    mPaused = false;
    mStarted = false;
}

void LTimer::start()
{
    //Start the timer
    mStarted = true;

    //Unpause the timer
    mPaused = false;

    //Get the current clock time
    mStartTicks = SDL_GetTicks();
	mPausedTicks = 0;
}

void LTimer::stop()
{
    //Stop the timer
    mStarted = false;

    //Unpause the timer
    mPaused = false;

	//Clear tick variables
	mStartTicks = 0;
	mPausedTicks = 0;
}

void LTimer::pause()
{
    //If the timer is running and isn't already paused
    if( mStarted && !mPaused )
    {
        //Pause the timer
        mPaused = true;

        //Calculate the paused ticks
        mPausedTicks = SDL_GetTicks() - mStartTicks;
		mStartTicks = 0;
    }
}

void LTimer::unpause()
{
    //If the timer is running and paused
    if( mStarted && mPaused )
    {
        //Unpause the timer
        mPaused = false;

        //Reset the starting ticks
        mStartTicks = SDL_GetTicks() - mPausedTicks;

        //Reset the paused ticks
        mPausedTicks = 0;
    }
}

Uint32 LTimer::getTicks()
{
	//The actual timer time
	Uint32 time = 0;

    //If the timer is running
    if( mStarted )
    {
        //If the timer is paused
        if( mPaused )
        {
            //Return the number of ticks when the timer was paused
            time = mPausedTicks;
        }
        else
        {
            //Return the current time minus the start time
            time = SDL_GetTicks() - mStartTicks;
        }
    }

    return time;
}

bool LTimer::isStarted()
{
	//Timer is running and paused or unpaused
    return mStarted;
}

bool LTimer::isPaused()
{
	//Timer is running and paused
    return mPaused && mStarted;
}

Dot::Dot()
{
	//Initializes the offsets
	mPosX = 0;
	mPosY = 0;
	
	//Initialize the velocity
	mVelX = 0;
	mVelY = 0;
}

void Dot::handleEvent(SDL_Event& e)
{
	//If a key was pressed
	if(e.type == SDL_KEYDOWN && e.key.repeat == 0)
	{
		//Adjust the velocity
		switch(e.key.keysym.sym)
		{
			case SDLK_UP: mVelY -= DOT_VEL; break;
			case SDLK_DOWN: mVelY += DOT_VEL; break;
			case SDLK_LEFT: mVelX -= DOT_VEL; break;
			case SDLK_RIGHT: mVelX += DOT_VEL; break;
		}
	}
	//If a key was released
	else if(e.type == SDL_KEYUP && e.key.repeat == 0)
	{
		//Adjust the velocity
		switch(e.key.keysym.sym)
		{
			case SDLK_UP: mVelY += DOT_VEL; break;
			case SDLK_DOWN: mVelY -= DOT_VEL; break;
			case SDLK_LEFT: mVelX += DOT_VEL; break;
			case SDLK_RIGHT: mVelX -= DOT_VEL; break;
		}
	}
}

void Dot::move()
{
	//Move the dot left or right
	mPosX += mVelX;
		
	//If the dot went too far to the left or right
	if((mPosX < 0) || (mPosX + DOT_WIDTH > LEVEL_WIDTH))
	{
		//Move back 
		mPosX -= mVelX;
	}
	
	//Move the dot up or down
	mPosY += mVelY;
		
	//If the dot went too far up or down
	if((mPosY < 0) || (mPosY + DOT_HEIGHT > LEVEL_HEIGHT))
	{
		//Move back
		mPosY -= mVelY;
	}
}

void Dot::render()
{
	//Show the dot relative to the camera
	//gDotTexture.render(mPosX, mPosY);
}

void Dot::shiftColliders()
{
	//Align collider to center of dot
	mCollider.x = mPosX;
	mCollider.y = mPosY;
}

LWindow::LWindow()
{
	//Initialize non-existant window
	mWindow = NULL;
	mMouseFocus = false;
	mKeyboardFocus = false;
	mFullScreen = false;
	mMinimized = false;
	mWidth = 0;
	mHeight = 0;
}

bool LWindow::init()
{
	//Create window
	mWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if(mWindow != NULL)
	{
		mMouseFocus = true;
		mKeyboardFocus = true;
		mWidth = SCREEN_WIDTH;
		mHeight = SCREEN_HEIGHT;
		
		//Create renderer for window
		mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		if(mRenderer == NULL)
		{
			printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
			SDL_DestroyWindow(mWindow);
			mWindow = NULL;
		}
		else
		{
			//Initialize renderer color
			SDL_SetRenderDrawColor(mRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
			
			//Grab window identifier
			mWindowID = SDL_GetWindowID(mWindow);
			
			//Flag as opened
			mShown = true;
		}
	}
	else
	{
		printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
	}
	
	return mWindow != NULL && mRenderer != NULL;
}

void LWindow::handleEvent(SDL_Event& e)
{
	//If an event was detected for this window
	if(e.type == SDL_WINDOWEVENT && e.window.windowID == mWindowID)
	{
		//Caption update flag
		bool updateCaption = false;
		switch(e.window.event)
		{
			//Window appeared
			case SDL_WINDOWEVENT_SHOWN:
				mShown = true;
				break;
				
			//Window disappeared
			case SDL_WINDOWEVENT_HIDDEN:
				mShown = false;
				break;
			
			//Get new dimensions and repaint on window size change
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				mWidth = e.window.data1;
				mHeight = e.window.data2;
				SDL_RenderPresent(gRenderer);
				break;
				
			//Repaint on exposure
			case SDL_WINDOWEVENT_EXPOSED:
				SDL_RenderPresent(gRenderer);
				break;
			
			//Mouse entered window
			case SDL_WINDOWEVENT_ENTER:
				mMouseFocus = true;
				updateCaption = true;
				break;
			
			//Mouse left window
			case SDL_WINDOWEVENT_LEAVE:
				mMouseFocus = false;
				updateCaption = true;
				break;
				
			//Window has keyboard focus
			case SDL_WINDOWEVENT_FOCUS_GAINED:
				mKeyboardFocus = true;
				updateCaption = true;
				break;
				
			//Window lost keyboard focus
			case SDL_WINDOWEVENT_FOCUS_LOST:
				mKeyboardFocus = false;
				updateCaption = true;
				break;
				
			//Window minimized
			case SDL_WINDOWEVENT_MINIMIZED:
				mMinimized = true;
				break;
				
			//Window maxmized
			case SDL_WINDOWEVENT_MAXIMIZED:
				mMinimized = false;
				break;
				
			//Window restored
			case SDL_WINDOWEVENT_RESTORED:
				mMinimized = false;
				break;
				
			//Hide on close
			case SDL_WINDOWEVENT_CLOSE:
				SDL_HideWindow(mWindow);
				break;				
		}
		
		//Update window caption with new data
		if(updateCaption)
		{
			std::stringstream caption;
			caption << "SDL Tutorial - MouseFocus:" << ((mMouseFocus) ? "On" : "Off") << " KeyboardFocus:"
				<< ((mKeyboardFocus) ? "On" : "Off");
			SDL_SetWindowTitle(mWindow, caption.str().c_str());
		}
	}
	//Enter exit full screen on return key
	else if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN)
	{
		if(mFullScreen)
		{
			SDL_SetWindowFullscreen(mWindow, SDL_FALSE);
			mFullScreen = false;
		}
		else
		{
			SDL_SetWindowFullscreen(mWindow, SDL_TRUE);
			mFullScreen = true;
			mMinimized = false;
		}
	}		
}

int LWindow::getWidth()
{
	return mWidth;
}

int LWindow::getHeight()
{
	return mHeight;
}

bool LWindow::hasMouseFocus()
{
	return mMouseFocus;
}

bool LWindow::hasKeyboardFocus()
{
	return mKeyboardFocus;
}

bool LWindow::isMinimized()
{
	return mMinimized;
}

bool LWindow::isShown()
{
	return mShown;
}

void LWindow::free()
{
	if( mWindow != NULL )
	{
		SDL_DestroyWindow( mWindow );
	}

	mMouseFocus = false;
	mKeyboardFocus = false;
	mWidth = 0;
	mHeight = 0;
}

void LWindow::focus()
{
	//Restore window if needed
	if(!mShown)
	{
		SDL_ShowWindow(mWindow);
	}
	
	//Move window forward_iterator
	SDL_RaiseWindow(mWindow);
}

void LWindow::render()
{
	if(!mMinimized)
	{
		//Clear screen
		SDL_SetRenderDrawColor(mRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(mRenderer);
		
		//Update screen
		SDL_RenderPresent(mRenderer);
	}
}

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
		{
			printf( "Warning: Linear texture filtering not enabled!" );
		}

		//Create window
		if(!gWindows[0].init())
		{
			printf("Window 0 could not be created!");
			success = false;
		}
		else
		{
			//Initialize renderer color
			SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

			//Initialize PNG loading
			int imgFlags = IMG_INIT_PNG;
			if( !( IMG_Init( imgFlags ) & imgFlags ) )
			{
				printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
				success = false;
			}

			 //Initialize SDL_ttf
			if( TTF_Init() == -1 )
			{
				printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
				success = false;
			}
		}
	}

	return success;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;
	
	return success;
}

void close()
{
	//Destroy windows
	for(int i = 0; i < TOTAL_WINDOWS; ++i)
	{
		gWindows[i].free();
	}
	
	//Free global font
	TTF_CloseFont( gFont );
	gFont = NULL;
	
	//Quit SDL subsystems
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

bool checkCollision( Circle& a, Circle& b)
{
	//Calculate total radius squared
	int totalRadiusSquared = a.r + b.r;
	totalRadiusSquared = totalRadiusSquared * totalRadiusSquared;
	
	//If the distance between the cecnters of the circles is less than the
	//sum of their radii
	if(distanceSquared(a.x, a.y, b.x, b.y) < (totalRadiusSquared))
	{
		//The circles have collided
		return true;
	}
	
	//If not
	return false;
}

bool checkCollision(Circle& a, SDL_Rect& b)
{
	//Closest point on collision box
	int cX, cY;
	
	//Find closest x offset
	if(a.x < b.x)
	{
		cX = b.x;
	}
	else if(a.x> b.x + b.w)
	{
		cX = b.x + b.w;
	}
	else
	{
		cX = a.x;
	}
	
	//Find closest y offset
	if(a.y < b.y)
	{
		cY = b.y;
	}
	else if(a.y> b.y + b.h)
	{
		cY = b.y + b.h;
	}
	else
	{
		cY = a.y;
	}
	
	//If the closest point is inside the circle
	if(distanceSquared(a.x, a.y, cX, cY) < a.r * a.r)
	{
		//This box and the circle have collided
		return true;
	}		
	
	//If the shapes have not collided
	return false;
}

double distanceSquared(int x1, int y1, int x2, int y2)
{
	int deltaX = x2 - x1;
	int deltaY = y2 - y1;
	return deltaX * deltaX + deltaY * deltaY;
}

int wmain( int argc, char* args[] )
{
	//Start up SDL and create window
	if( !init() )
	{
		printf( "Failed to initialize!\n" );
	}
	else
	{
		//Load media
		if( !loadMedia() )
		{
			printf( "Failed to load media!\n" );
		}
		else
		{	
			//Initialize the rest of the windows
			for(int i = 1; i < TOTAL_WINDOWS; ++i)
			{
				gWindows[i].init();
			}
	
			//Main loop flag
			bool quit = false;

			//Event handler
			SDL_Event e;
			
			//While application is running
			while(!quit)
			{
				//Handle events on queue
				while(SDL_PollEvent(&e) != 0)
				{
					//User requests quit
					if(e.type == SDL_QUIT)
					{
						quit = true;
					}
					
					//Handle window events
					for(int i = 0; i < TOTAL_WINDOWS; ++i)		
					{
						gWindows[i].handleEvent(e);
					}
					
					//Pull up window
					if(e.type == SDL_KEYDOWN)
					{
						switch(e.key.keysym.sym)
						{
							case SDLK_1:
								gWindows[0].focus();
								break;
							case SDLK_2:
								gWindows[1].focus();
								break;
							case SDLK_3:
								gWindows[2].focus();
								break;
						}
					}
				}
				
				//Update all windows
				for(int i = 0; i < TOTAL_WINDOWS; ++i)
				{
					gWindows[i].render();
				}
				
				//Check all windows
				bool allWindowsClosed = true;
				for(int i = 0; i < TOTAL_WINDOWS; ++i)
				{
					if(gWindows[i].isShown())
					{
						allWindowsClosed = false;
						break;
					}
				}
				
				//Application closed all windows
				if(allWindowsClosed)
				{
					quit = true;
				}
			}
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}