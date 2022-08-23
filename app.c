#include <stdio.h>
#include <pthread.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

/* Local includes. */
#include "console.h"

#define TASK1_PRIORITY 1
#define TASK2_PRIORITY 1
#define TASK3_PRIORITY 2

#define BLACK "\033[30m" /* Black */
#define RED "\033[31m"   /* Red */
#define GREEN "\033[32m" /* Green */
#define DISABLE_CURSOR() printf("\e[?25l")
#define ENABLE_CURSOR() printf("\e[?25h")

#define clear() printf("\033[H\033[J")
#define gotoxy(x, y) printf("\033[%d;%dH", (y), (x))

typedef struct
{
    int pos;
    char *color;
    int flag_acesso;
    int flag_apagado;
    int period_ms;
} st_led_param_t;

st_led_param_t green = {
    6,
    GREEN,
    0,
    0,
    250};
 
st_led_param_t red = {
    12,
    RED,
    0,
    1,
    100};

xTaskHandle notified_hdlr = NULL;
TaskHandle_t greenTask_hdlr,redTask_hdlr;

static void prvTask_led(void *pvParameters)
{
    // pvParameters contains LED params
    st_led_param_t *led = (st_led_param_t *)pvParameters;
    portTickType xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        // console_print("@");
        gotoxy(led->pos, 2);
        printf("%s⬤", led->color);
        fflush(stdout);
           
        if(led->flag_acesso==1)
        {
           led->flag_acesso=0;
            vTaskSuspend(NULL);            
        }

        vTaskDelay(led->period_ms / portTICK_PERIOD_MS);
        // vTaskDelayUntil(&xLastWakeTime, led->period_ms / portTICK_PERIOD_MS);

        gotoxy(led->pos, 2);
        printf("%s ", BLACK);
        fflush(stdout);

        if(led->flag_apagado==1)
        {
            xTaskNotify(notified_hdlr, 0UL, eSetValueWithOverwrite);           
            vTaskSuspend(NULL);
        }

        //xTaskNotify(notified_hdlr, 0UL, eSetValueWithOverwrite);     
        vTaskDelay(led->period_ms / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}

static void prvTask_Notified(void *pvParameters)
{
    uint32_t notificationValue;

    for (;;)
    {
        if (xTaskNotifyWait(
                ULONG_MAX,
                ULONG_MAX,
                &notificationValue,
                portMAX_DELAY))
        {
            gotoxy(2, 4);
            printf("%s ", GREEN);
            if (notificationValue == 1)
            {
                printf("Tecla Acionada \n");
            
            }
            else
            {
                printf("               \n");
            }
        }
    }
    vTaskDelete(NULL);
}

#include <termios.h>

static void prvTask_getChar(void *pvParameters)
{
    char key;
    int n;

    /* I need to change  the keyboard behavior to
    enable nonblock getchar */
    struct termios initial_settings,
        new_settings;

    tcgetattr(0, &initial_settings);

    new_settings = initial_settings;
    new_settings.c_lflag &= ~ICANON;
    new_settings.c_lflag &= ~ECHO;
    new_settings.c_lflag &= ~ISIG;
    new_settings.c_cc[VMIN] = 0;
    new_settings.c_cc[VTIME] = 1;

    tcsetattr(0, TCSANOW, &new_settings);
    /* End of keyboard configuration */
    for (;;)
    {
        int stop = 0;
        key = getchar();

        switch (key)
        {
       
         case 'k':
            stop = 1;
            xTaskNotify(notified_hdlr, 01UL, eSetValueWithOverwrite);    
            break;

         default:
            xTaskNotify(notified_hdlr, 00UL, eSetValueWithOverwrite);
         break;
        }
        if (stop)
        {
            break;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    tcsetattr(0, TCSANOW, &initial_settings);
    ENABLE_CURSOR();
    exit(0);
    vTaskDelete(NULL);
}



void app_run(void)
{

    clear();
    DISABLE_CURSOR();
    printf(
        "╔═════════════════╗\n"
        "║                 ║\n"
        "╚═════════════════╝\n");

    xTaskCreate(prvTask_Notified, "Notified", configMINIMAL_STACK_SIZE, NULL, TASK2_PRIORITY, &notified_hdlr);
    xTaskCreate(prvTask_led, "LED_green", configMINIMAL_STACK_SIZE, &green, TASK1_PRIORITY,  &greenTask_hdlr);
    xTaskCreate(prvTask_led, "LED_Red  ", configMINIMAL_STACK_SIZE, &red, TASK1_PRIORITY,  &redTask_hdlr);
    xTaskCreate(prvTask_getChar, "Get_key", configMINIMAL_STACK_SIZE, NULL, TASK3_PRIORITY, NULL);


    
    /* Start the tasks and timer running. */
    vTaskStartScheduler();
    vTaskSuspend(&redTask_hdlr);

    /* If all is well, the scheduler will now be running, and the following
     * line will never be reached.  If the following line does execute, then
     * there was insufficient FreeRTOS heap memory available for the idle and/or
     * timer tasks      to be created.  See the memory management section on the
     * FreeRTOS web site for more details. */
    for (;;)
    {
    }
}