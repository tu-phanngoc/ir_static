#ifndef GRAPHICS_H
#define GRAPHICS_H

unsigned char*  getDisplay(void);

void drawArrayImage(int x, int yBlock, unsigned char* buffer, int width, int height);
void clearScreen(void);
void drawNumber(int yBlock, unsigned char* number);
void drawDistance(int yBlock, unsigned char* number);
void drawString(int yBlock, unsigned char* disp);
void rotate(void);
void drawSleep(void);
void drawResetAsking(void);
void drawPairing(void);
void drawNavigationScreen(void);
void drawApproachingTurnPoint(void);
void drawApproachingPoi(void);
void drawInfoScreen(void);
void drawCompassScreen(void);
void drawNavigation10mScreen(void);
void setTime(unsigned char* val);
void setRoad(unsigned char* val);
void setNextRoad(unsigned char* val);
void setAltitude(unsigned char* val);
void setDtnt(int val);
void setNextTurn(int val);
void setDtp(int val);
void setPar(int val);
void setIcon(int val);
void setBattery(int val);
void setCommpass(int val);
void drawReset(void);
void passkeySet(char *passkey);
#endif

