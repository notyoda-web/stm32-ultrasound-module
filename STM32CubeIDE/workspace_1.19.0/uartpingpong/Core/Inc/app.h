#ifndef APP_H
#define APP_H
void app_init(void);   /* call once, after all MX_..._Init() */
void app_loop(void);   /* call repeatedly from while(1)      */
#endif
