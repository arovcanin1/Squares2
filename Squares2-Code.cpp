#include "mbed.h"
#include "stm32f413h_discovery_ts.h"
#include "stm32f413h_discovery_lcd.h"
#include <stdlib.h>   
#include <time.h>
#include <vector>
#include <typeinfo>
#include <string>

TS_StateTypeDef TS_State = { 0 };

void losingScreenView(int score);
void gameView();


bool playPage;
bool helpPage;
bool losingPage;
bool startPage;
int score;
short mySquareLength = 12;
int timing;

void setMySquareLength(short len)  {
    mySquareLength = len; 
}


void drawSquare(short x, short y, short length, uint16_t color)
{
    BSP_LCD_SetTextColor(color);
    BSP_LCD_FillRect (x, y, length, length);
}

void drawCircle(short x, short y, short length, uint16_t color)
{
    BSP_LCD_SetTextColor(color);
    BSP_LCD_FillCircle (x, y, length);
}

void removeSquare(short x, short y, short length)
{
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_FillRect (x, y, length, length);
}

void removeCircle(short x, short y, short length)
{
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_FillCircle (x, y, length);
}

bool touchedSquare(short x, short y, short myX, short myY, int len, int myLength)
{
    return ((myX-myLength/2 <= x+len/2 && 
      myX-myLength/2 >= x-len/2 && 
      myY+myLength/2 >= y-len/2 &&
      myY+myLength/2 <= y+len/2) || //myBottomLeft touching TopRight
     (myX+myLength/2 >= x-len/2 && 
      myX+myLength/2 <= x+len/2 && 
      myY+myLength/2 >= y-len/2 &&
      myY+myLength/2 <= y+len/2) || //myBottomRight touching TopLeft
     (myX+myLength/2 >= x-len/2 && 
      myX+myLength/2 <= x+len/2 && 
      myY-myLength/2 <= y+len/2 &&
      myY-myLength/2 >= y-len/2) || //myTopRight touching BottomLeft
     (myX-myLength/2 <= x+len/2 && 
      myX-myLength/2 >= x-len/2 && 
      myY-myLength/2 <= y+len/2 &&
      myY-myLength/2 >= y-len/2));  //myTopLeft touching BottomRight

}

bool touchedCircle(short x, short y, short myX, short myY, int len)
{
    return ( (myX+mySquareLength/2-x)*(myX+mySquareLength/2-x) + 
             (myY+mySquareLength/2-y)*(myY+mySquareLength/2-y) <= len*len ||
             (myX-mySquareLength/2-x)*(myX-mySquareLength/2-x) + 
             (myY+mySquareLength/2-y)*(myY+mySquareLength/2-y) <= len*len ||
             (myX+mySquareLength/2-x)*(myX+mySquareLength/2-x) + 
             (myY-mySquareLength/2-y)*(myY-mySquareLength/2-y) <= len*len ||
             (myX-mySquareLength/2-x)*(myX-mySquareLength/2-x) + 
             (myY-mySquareLength/2-y)*(myY-mySquareLength/2-y) <= len*len 
            ); 
	    // any square point touching circle
}


class Shape
{   
public:
    virtual bool isHit() = 0;
    virtual bool isDeadly() = 0;
    virtual void move(short displacement) = 0;
    virtual void disappear(short a, short b, short myLenght) = 0;
};

class RedSquare : public Shape
{

	short x, y, length;
	bool hit, deadly;
	short direction;
	/* 1- left-right, 2- right-left, 3- up-down, 4- down-up*/
	short speed, acceleration;
    
    public:
	RedSquare()
	{
	    x = rand() % 240 + 1;
	    y = rand() % 240 + 1;
	    length = rand() % 12 + 4;
	    int dir = rand() % 4 + 1;
		direction = dir;
        deadly=true;
        speed = rand()%6 + 1;
        if (score<=500) acceleration=1;
        else acceleration = score/500;

		if (direction==1)
			x = 0;
		if (direction == 2)
			x = 250 - length;
		if (direction == 3) 
		    y = 0;
		if (direction == 4)
		    y = 250 - length;
		drawSquare(x, y, length, LCD_COLOR_RED);
	}
	bool isHit()
	{
		return hit;
	}
	
	bool isDeadly() 
	{
	    return deadly;
	}
	
	void move(short displacement)
	{
		removeSquare(x,y, length);
		
		if (direction == 1) {
		    x+=displacement*speed*acceleration;
		}
		else if (direction == 2) {
		    x-= displacement*speed*acceleration;
		}
		else if (direction == 3) {
		    y+= displacement*speed*acceleration;
		}
		else if (direction == 4) {
		    y-= displacement*speed*acceleration;
		}
		
		drawSquare(x,y,length, LCD_COLOR_RED);
		
	}
	void disappear(short a, short b, short myLength)
	{
		if (touchedSquare(x, y, a, b, length, myLength)) {
			removeSquare(x, y, length);
			hit = true;
		}
		else {
		    hit = false;
		}
	}
	friend void drawSquare(short x, short y, short length, uint16_t color);
	friend void removeSquare(short x, short y, short length);
	friend bool touchedSquare(short x, short y, short a, short b, short length, short myLength);
};


class BlackSquare : public Shape
{

	short x, y, length;
	bool hit, deadly;
	short direction;
	/* 1- left-right, 2- right-left, 3- up-down, 4- down-up*/
	short speed, acceleration;
    
    public:
	BlackSquare()
	{
	    x = rand() % 240 + 1;
	    y = rand() % 240 + 1;
	    length = rand() % 12 + 4;
	    int dir = rand() % 4 + 1;
		direction = dir;
		deadly=false;
		
        speed = rand()%6 + 1;
        if (score<=500) acceleration=1;
        else acceleration = score/500;
        
		if (direction==1)
			x = 0;
		if (direction == 2)
			x = 250 - length;
		if (direction == 3) 
		    y = 0;
		if (direction == 4)
		    y = 250 - length;
		drawSquare(x, y, length, LCD_COLOR_BLACK);
	}
	bool isHit()
	{
		return hit;
	}
	
	bool isDeadly() 
	{
	    return deadly;
	}
	
	void move(short displacement)
	{
		removeSquare(x,y, length);
		
		if (direction == 1) {
		    x+= displacement*speed*acceleration;
		}
		else if (direction == 2) {
		    x-= displacement*speed*acceleration;
		}
		else if (direction == 3) {
		    y+= displacement*speed*acceleration;
		}
		else if (direction == 4) {
		    y-= displacement*speed*acceleration;
		}
		
		drawSquare(x,y,length, LCD_COLOR_BLACK);
	}
	
	void disappear(short a, short b, short myLength)
	{
		if (touchedSquare(x, y, a, b, length, myLength)) {
			removeSquare(x, y, length);
			hit = true;
			score += 50;
		}
		else {
		    hit = false;
		}
	}
	friend void drawSquare(short x, short y, short length, uint16_t color);
	friend void removeSquare(short x, short y, short length);
	friend bool touchedSquare(short x, short y, short a, short b, short length, short myLength);
};

class BlackCircle : public Shape
{

	short x, y, length;
	bool hit, bigger, deadly;
	short direction;
	/* 1- left-right, 2- right-left, 3- up-down, 4- down-up*/
	short speed, acceleration;
    
    public:
	BlackCircle()
	{
	    x = rand() % 240 + 1;
	    y = rand() % 240 + 1;
	    length = 8;
	    int dir = rand() % 4 + 1;
		direction = dir;
		deadly=false;
        
		speed = rand()%6 + 1;
        if (score<=500) acceleration=1;
        else acceleration = score/500;
        
		if (direction==1)
			x = 0;
		if (direction == 2)
			x = 250 - length;
		if (direction == 3) 
		    y = 0;
		if (direction == 4)
		    y = 250 - length;
		drawCircle(x, y, length, LCD_COLOR_BLACK);
	}
	bool isHit()
	{
		return hit;
	}
	
	bool isDeadly() 
	{
	    return deadly;
	}
	
	void move(short displacement)
	{
		removeCircle(x,y, length);
		
		if (direction == 1) {
		    x+= displacement*speed*acceleration;
		}
		else if (direction == 2) {
		    x-= displacement*speed*acceleration;
		}
		else if (direction == 3) {
		    y+= displacement*speed*acceleration;
		}
		else if (direction == 4) {
		    y-= displacement*speed*acceleration;
		}
		
		drawCircle(x,y,length, LCD_COLOR_BLACK);
		
	}
	void disappear(short a, short b, short myLength)
	{
		if (touchedCircle(x, y, a, b, length)) {
			removeCircle(x, y, length);
			hit = true;
	        bigger = rand() % 2 ;
	        if (bigger==1) {
	            setMySquareLength(16);
	        }
	        else {
	            setMySquareLength(12);
	            removeSquare(x,y,16);
	        }
			score += 1000;
		}
		else {
		    hit = false;
		}
	}
	friend void drawCircle(short x, short y, short length, uint16_t color);
	friend void removeCircle(short x, short y, short length);
	friend bool touchedCircle(short x, short y, short a, short b, short length);
	friend void setMySquareLength();
};

class RedCircle : public Shape
{

	short x, y, length;
	bool hit, smaller, deadly;
	short direction;
	/* 1- left-right, 2- right-left, 3- up-down, 4- down-up*/
	short speed, acceleration;
    
    public:
	RedCircle()
	{
	    x = rand() % 240 + 1;
	    y = rand() % 240 + 1;
	    length = 8;
	    int dir = rand() % 4 + 1;
		direction = dir;
		deadly=false;
		
		speed = rand()%6 + 1;
        if (score<=500) acceleration=1;
        else acceleration = score/500;
        
		if (direction==1)
			x = 0;
		if (direction == 2)
			x = 250 - length;
		if (direction == 3) 
		    y = 0;
		if (direction == 4)
		    y = 250 - length;
		drawCircle(x, y, length, LCD_COLOR_RED);
	}
	bool isHit()
	{
		return hit;
	}
	
	bool isDeadly() 
	{
	    return deadly;
	}
	
	void move(short displacement)
	{
		removeCircle(x,y, length);
		
		if (direction == 1) {
		    x+=displacement*speed*acceleration;
		}
		else if (direction == 2) {
		    x-= displacement*speed*acceleration;
		}
		else if (direction == 3) {
		    y+= displacement*speed*acceleration;
		}
		else if (direction == 4) {
		    y-= displacement*speed*acceleration;
		}
		
		drawCircle(x,y,length, LCD_COLOR_RED);
	}
	
	void disappear(short a, short b, short myLength)
	{
		if (touchedCircle(x, y, a, b, length)) {
	        smaller = rand() % 2 ;
	        if (smaller==1) {
	            setMySquareLength(7);
	            removeSquare(x, y, 16);
	        }
	        else {
	            setMySquareLength(12);
	            removeSquare(x, y, 16);
	        }
			removeCircle(x, y, length);
			hit = true;
			score -= 1000;
			
			
		}
		else {
		    hit = false;
		}
	}
	friend void drawCircle(short x, short y, short length, uint16_t color);
	friend void removeCircle(short x, short y, short length);
	friend bool touchedCircle(short x, short y, short a, short b, short length);
	friend void setMySquareLength();
};

class MySquare
{
	short xH, yH, xV, yV, length;
public:
	MySquare(short a, short b, short c, short d)
	{
		xH = a;
		yH = b;
		xV = c;
		yV = d;
		show();
	}
	void setX(short x)
	{
		xH = x-length/2;
		xV = x;
	}
	void setY(short y)
	{
		yH = y;
		yV = y - length/2;
	}
	
	void setLength (int len) {
	    length = len;
	}
	
	void move(short speed)
	{
		hide();
		if (xH + 9 < 240) {
			xH += speed;
			xV += speed;
		}
		if (xH > 0) {
			xH -= speed;
			xV -= speed;
		}
		if (yV > 0) {
			yH -= speed;
			yV -= speed;
		}
		if (yV + 9 < 210) {
			yH += speed;
			yV += speed;
		}
		show();
	}
	void hide()
	{
		BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
		BSP_LCD_FillRect(xH, yH, mySquareLength, mySquareLength);
	}
	void show()
	{
		BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
		BSP_LCD_FillRect(xH, yH, mySquareLength, mySquareLength);
	}
	
};

void addSecond()
{
	timing++;
}


void showScore(int score) { 
    /*Score description */
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
    BSP_LCD_SetFont(&Font12);
    uint8_t scoreString[50] = {0};
    sprintf((char *) scoreString, "Your score: %d ", score);
    BSP_LCD_DisplayStringAt(0, 5, scoreString, CENTER_MODE);
}


void losingScreenView(int score) {
    
    BSP_LCD_Clear(LCD_COLOR_WHITE);
   // BSP_TS_GetState(&TS_State);
    /* Set Touchscreen Title description */
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_SetFont(&Font24);
    BSP_LCD_DisplayStringAt(0, 35, (uint8_t *)"You lost", CENTER_MODE);
    
   /* Set Touchscreen Score description */
    uint8_t scoreString[50] = {0};
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 80, (uint8_t *)"Final score:", CENTER_MODE);
    BSP_LCD_SetFont(&Font24);
    sprintf((char *) scoreString, "%d", score);
    BSP_LCD_DisplayStringAt(0, 120, scoreString, CENTER_MODE);
    
    /* Set Touchscreen Help button */
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_FillRect(0, 170, BSP_LCD_GetXSize(), 40);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 185, (uint8_t *)"Try again", CENTER_MODE);
    
    /*Decoration*/
    BSP_LCD_SetTextColor (LCD_COLOR_BLACK);
    BSP_LCD_FillRect (10, 15, 8, 8);
    BSP_LCD_FillRect (100, 60, 5, 5);
    BSP_LCD_FillRect (210, 30, 10, 10);
    BSP_LCD_SetTextColor (LCD_COLOR_RED);
    BSP_LCD_FillRect (120, 10, 5, 5);
    BSP_LCD_FillRect (20, 90, 7, 7);
    BSP_LCD_FillRect (200, 70, 6, 6);
    BSP_LCD_SetTextColor (LCD_COLOR_BLACK);
    BSP_LCD_FillRect (210, 120, 4, 4);
    BSP_LCD_FillRect (60, 130, 5, 5);
    BSP_LCD_FillRect (180, 150, 6, 6);
    BSP_LCD_SetTextColor (LCD_COLOR_RED);
    BSP_LCD_FillRect (10, 155, 5, 5);
    BSP_LCD_FillRect (160, 120, 4, 4);
    
    while(1) {
        BSP_TS_GetState(&TS_State);
        if(TS_State.touchDetected) {
    	
    		uint16_t y = TS_State.touchY[0];
    		uint16_t x = TS_State.touchX[0];
    		if (x > 0 && x < BSP_LCD_GetXSize() && y > 170 && y < 210) {
         		losingPage = false;
    			startPage = false;
    			playPage = true;
    			helpPage = false;
    			break;
    		}
        }
        wait_ms(100);
    }
    

}


void gameView () {
    
    BSP_LCD_Clear(LCD_COLOR_WHITE);
    showScore(score);
    std::vector<Shape*> shapes;
    
    Ticker t;
	t.attach(&addSecond, 1);
	
	MySquare ms(120, 120, 120, 120);
	        
	    ms.move(1);
	    losingPage=false;
	    while (1) {
	        
	        ms.setLength(mySquareLength);
	        addSecond();
	        
	        if (timing%15==0) {
    	        shapes.push_back(new RedSquare());
    	        shapes.push_back(new BlackSquare());
    	        shapes.push_back(new RedCircle());
        	    shapes.push_back(new BlackCircle());
	        }
	        
	        short x1=120,y1=120;
	        BSP_TS_GetState(&TS_State);
			if(TS_State.touchDetected) {
				x1 = TS_State.touchX[0];
				y1 = TS_State.touchY[0];
				ms.hide();
				ms.setX(x1);
				ms.setY(y1);
				ms.show();
			}
			for (int i=0; i<shapes.size(); i++) {
	            shapes[i]->move(1);
	            shapes[i]->disappear(x1,y1,mySquareLength);
	            
	            if (shapes[i]->isHit() && shapes[i]->isDeadly()) {
	                losingScreenView(score);
	                losingPage = true;
	                playPage = false;
	                break;
	            }
	            if (shapes[i]->isHit()) {
					shapes.erase(shapes.begin() + i);
	            }
	            showScore(score);
	            
	        }
	        
	        if (losingPage==true) {
	            score = 0;
	            mySquareLength=12;
	            break;
	        }
	        wait_ms(100);
	    }
	    
	   
}

void homeScreenView() {
    
    BSP_LCD_Clear(LCD_COLOR_WHITE);
    /* Set Touchscreen Title description */
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_SetFont(&Font24);
    BSP_LCD_DisplayStringAt(0, 35, (uint8_t *)"Squares 2", CENTER_MODE);
    
    /* Set Touchscreen Play button */
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_FillRect(0, 110, BSP_LCD_GetXSize(), 40);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 125, (uint8_t *)"Play", CENTER_MODE);
    
    /* Set Touchscreen Help button */
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_FillRect(0, 170, BSP_LCD_GetXSize(), 40);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 185, (uint8_t *)"Help", CENTER_MODE);
    
    /*Decoration*/
    BSP_LCD_SetTextColor (LCD_COLOR_RED);
    BSP_LCD_FillRect (10, 15, 8, 8);
    BSP_LCD_FillRect (100, 70, 5, 5);
    BSP_LCD_FillRect (210, 30, 10, 10);
    BSP_LCD_SetTextColor (LCD_COLOR_BLACK);
    BSP_LCD_FillRect (120, 10, 5, 5);
    BSP_LCD_FillRect (40, 90, 7, 7);
    BSP_LCD_FillRect (190, 80, 6, 6);
    
    while(1) {
        BSP_TS_GetState(&TS_State);
	if(TS_State.touchDetected) {
		uint16_t x = TS_State.touchX[0];
		uint16_t y = TS_State.touchY[0];

                if(x > 0 && x < BSP_LCD_GetXSize() && y > 170 && y < 210) {
			score = 0;
			helpPage = true;
			playPage = false;
			startPage = false;
			break;
		} 
		if(x > 0 && x < BSP_LCD_GetXSize() && y > 110 && y < 150) {
			score = 0;
			playPage = true;
			helpPage = false;
			startPage = false;
			break;
		}
	}
        wait_ms(100);
    }
}


void helpScreenView() {
    
    BSP_LCD_Clear(LCD_COLOR_WHITE);
    
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_DisplayStringAt(0, 10, (uint8_t *)"Help", CENTER_MODE);
    
    /*Back*/
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_DisplayStringAt(2, 3, (uint8_t *)"<--", LEFT_MODE);
    
        
    /*Items*/
        
    BSP_LCD_SetTextColor (LCD_COLOR_BLACK);
    BSP_LCD_FillRect (12, 50, 15, 15);
    BSP_LCD_FillRect (15, 80, 10, 10);
    BSP_LCD_FillCircle (20, 115, 6);
    BSP_LCD_SetTextColor (LCD_COLOR_RED);
    BSP_LCD_FillRect (15, 140, 10, 10);
    BSP_LCD_FillCircle (20, 170, 6);
        
    /*Explanations*/
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_SetFont(&Font8);
    BSP_LCD_DisplayStringAt(0, 50, (uint8_t *)"This is your square.", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, 60, (uint8_t *)"It is controlled by mouse.", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, 80, (uint8_t *)"This one will give you points.", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, 90, (uint8_t *)"It will make your square bigger.", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, 110, (uint8_t *)"This is powerup.", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, 120, (uint8_t *)"If you hit it, you will get 1000p.", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, 140, (uint8_t *)"This is an enemy.", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, 150, (uint8_t *)"If you hit this one, you lose.", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, 170, (uint8_t *)"This is powerdown.", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, 180, (uint8_t *)"If you hit it, you will lose 1000p.", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, 200, (uint8_t *)"For powerups, besides getting points,", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, 210, (uint8_t *)"your square will sometimes get 40% bigger,", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, 220, (uint8_t *)"and for powerdowns, besides losing points,", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, 230, (uint8_t *)"it will sometimes get 40% smaller.", CENTER_MODE);
    
    while(1) {
        BSP_TS_GetState(&TS_State);
        if(TS_State.touchDetected) {
    	
    		uint16_t y = TS_State.touchY[0];
    		uint16_t x = TS_State.touchX[0];
    		if (x>=0 && x<=40 && y>=2 && y<=15) {
    			helpPage = false;
    			startPage = true;
    			playPage = false;
    			break;
    		}
        }
        wait_ms(100);
    }
        
    
}

int main() {
    
    BSP_LCD_Init();

    /* Touchscreen initialization */
    if (BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize()) == TS_ERROR) {
        printf("BSP_TS_Init error\n");
    }

    /* Clear the LCD */
    BSP_LCD_Clear(LCD_COLOR_WHITE);
    
    srand (time(NULL));
	 
    startPage = true;
    helpPage = false;
    playPage = false;
    losingPage = false;
    

    
    while (1) {
         if (startPage == true) homeScreenView();
         else if (helpPage == true) helpScreenView();
         else gameView();
         wait_ms(10);
   }
}



