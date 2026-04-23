#ifndef MENU_H
#define MENU_H

void initMenu();
void renderMenu();
void updateMenu();
void handleMenuInput(unsigned char tecla);
void handleMenuSpecial(int tecla);
void handleMenuMouse(int button, int state, int x, int y);

#endif
 
