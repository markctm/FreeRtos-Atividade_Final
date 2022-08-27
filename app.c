/* Kernel includes. */

#include <stdio.h>
#include <pthread.h>
#include <termios.h>

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
#define TASK4_PRIORITY 2
#define TASK5_PRIORITY 3
#define TASK6_PRIORITY 3

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
TaskHandle_t greenTask_hdlr,redTask_hdlr,prvTask_logger_hldr,prvTask_coder_hldr,loggerTexto_Task_hdlr;
QueueHandle_t structQueue = NULL;
QueueHandle_t structQueue2 = NULL;
QueueHandle_t structQueue3 = NULL;
QueueHandle_t structQueue4 = NULL;

static void prvTask_logger(void *pvParameters);
static void prvTask_coder(void *pvParameters);
static void transmit_char(char *chr);


static void prvTask_led(void *pvParameters)
{
    // pvParameters contains LED params
    st_led_param_t *led = (st_led_param_t *)pvParameters;
    portTickType xLastWakeTime = xTaskGetTickCount();
    char key;

    for (;;)
    {
              
        if (xQueueReceive(structQueue4, &key, pdMS_TO_TICKS(2000)) == pdPASS)
        {
            
            if(key=='.')
            {
                gotoxy(led->pos, 2);
                printf("%s⬤", led->color);
                fflush(stdout);              
                vTaskDelay(100 / portTICK_PERIOD_MS);

            }

            if(key=='-')
            {
                gotoxy(led->pos, 2);
                printf("%s⬤", led->color);
                fflush(stdout); 
                
                vTaskDelay(350 / portTICK_PERIOD_MS);
            }

            gotoxy(led->pos, 2);
            printf("%s ", BLACK);
            fflush(stdout);
            vTaskDelay(500 / portTICK_PERIOD_MS);
            
            //}
             

        }
        else{

            printf("", led->color);
            vTaskDelete(NULL);

        }

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
                xTaskCreate(prvTask_logger, "Logger", configMINIMAL_STACK_SIZE, NULL, TASK4_PRIORITY, &prvTask_logger_hldr);
            }
            if (notificationValue == 2)
            {
                xTaskCreate(prvTask_coder, "Coder", configMINIMAL_STACK_SIZE, 8, TASK4_PRIORITY, &prvTask_coder_hldr);
                xTaskCreate(prvTask_led, "LED_Red  ", configMINIMAL_STACK_SIZE, &red, TASK6_PRIORITY,  &redTask_hdlr);
            }
                      
            else
            {
                printf("               \n");
            }
        }
    }
    vTaskDelete(NULL);
}



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
        if((key==10)||(key==42)||(key==32)||((key>=97)&&(key<=122))||((key>=48)&&(key<=57))) //Filtro de Caracteres Permitidos
        {

            switch (key)
            {
                case '*':
                stop = 1;
                break;

                case 10:
                    
                    gotoxy(0, 5);
                    printf("                                                                                           \n");
                    xTaskNotify(notified_hdlr, 1UL, eSetValueWithOverwrite);

                break;

                default:
                if (xQueueSend(structQueue, &key, 0) != pdTRUE)
                {       
                }

                if (xQueueSend(structQueue3, &key, 0) != pdTRUE)
                {       
                }
                break;
            }
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


static void transmit_char(char *chr)
{

    if (xQueueSend(structQueue2, &chr, 0) != pdTRUE)
    {
     //   printf("Error");
    }

   if (xQueueSend(structQueue4, &chr, 0) != pdTRUE)
   {
      //  print("Error");
    }
}


static void prvTask_logger(void *pvParameters)
{
    (void)pvParameters;
    char key;
    char caracter;
    const int CURSOR_Y = 4;
    int cursor_x = 0;
    for (;;)
    {      

        if (xQueueReceive(structQueue, &key, pdMS_TO_TICKS(400)) == pdPASS)
        {
            switch(key)
                {
                    case 'a':
                    transmit_char((char *)'.');
                    transmit_char((char *)'-');
                    transmit_char((char *)' ');                      
                    break;
                    case 'b':

                    transmit_char((char *)'-');
                    transmit_char((char *)'.');
                    transmit_char((char *)'.');  
                    transmit_char((char *)'.');
                    transmit_char((char *)' ');     

                    break;
                    case 'c':
                    transmit_char((char *)'-');
                    transmit_char((char *)'.');
                    transmit_char((char *)'-');  
                    transmit_char((char *)'.');
                    transmit_char((char *)' ');     

                    break;
                    case 'd':
                                 
                    transmit_char((char *)'-');
                    transmit_char((char *)'.');  
                    transmit_char((char *)'.');
                    transmit_char((char *)' ');     
                    break;
                    case 'e':

                    transmit_char((char *)'.');
                    transmit_char((char *)' ');

                    break;
                    case 'f':

                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)'-');
                    transmit_char((char *)'.');
                    transmit_char((char *)' ');

                    break;
                    case 'g':

                    
                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)' ');

                    break;
                    case 'h':


                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)' ');

                    break;
                    case 'i':

                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)' ');

                    break;
                    case 'j':

                    transmit_char((char *)'.');
                    transmit_char((char *)'-');
                    transmit_char((char *)'-');
                    transmit_char((char *)'-');
                    transmit_char((char *)' ');

                    break;
                    case 'k':

                    transmit_char((char *)'-');
                    transmit_char((char *)'.');
                    transmit_char((char *)'-');
                    transmit_char((char *)' ');

                    break;
                    case 'l':

                    transmit_char((char *)'.');
                    transmit_char((char *)'-');
                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)' ');

                    break;
                    case 'm':


                    transmit_char((char *)'-');
                    transmit_char((char *)'-');
                    transmit_char((char *)' ');


                    break;
                    case 'n':

                    transmit_char((char *)'-');
                    transmit_char((char *)'.');
                    transmit_char((char *)' ');

                    break;
                    case 'o':

                    transmit_char((char *)'-');
                    transmit_char((char *)'-');
                    transmit_char((char *)'-');
                    transmit_char((char *)' ');

                    break;
                    case 'p':

                    transmit_char((char *)'.');
                    transmit_char((char *)'-');
                    transmit_char((char *)'-');
                    transmit_char((char *)'.');
                    transmit_char((char *)' ');

                    break;
                    case 'q':


                    transmit_char((char *)'-');
                    transmit_char((char *)'-');
                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)' ');

                    break;
                    case 'r':

                    transmit_char((char *)'.');
                    transmit_char((char *)'-');
                    transmit_char((char *)'.');
                    transmit_char((char *)' ');

                    break;
                    case 's':

                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)' ');

                    break;
                    case 't':

                    transmit_char((char *)'-');
                    transmit_char((char *)' ');

                    break;
                    case 'u':

                    transmit_char((char *)'.');
                    transmit_char((char *)'-');
                    transmit_char((char *)'-');
                    transmit_char((char *)' ');


                    break;
                    case 'v':

                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)'-');
                    transmit_char((char *)' ');

                    break;
                    case 'w':

                    transmit_char((char *)'.');
                    transmit_char((char *)'-');
                    transmit_char((char *)'-');
                    transmit_char((char *)' ');
                   
                    break;
                    case 'x':

                    transmit_char((char *)'-');
                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)'-');
                    transmit_char((char *)' ');

                    break;                  
                    case 'y':

                    transmit_char((char *)'-');
                    transmit_char((char *)'.');
                    transmit_char((char *)'-');
                    transmit_char((char *)'-');
                    transmit_char((char *)' ');
                    break;
                    case 'z':

                    transmit_char((char *)'-');
                    transmit_char((char *)'-');
                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)' ');

                    break;
                    case '0':

                    transmit_char((char *)'-');
                    transmit_char((char *)'-');
                    transmit_char((char *)'-');
                    transmit_char((char *)'-');
                    transmit_char((char *)'-');
                    transmit_char((char *)' ');

                    break;
                    case '1':

                    transmit_char((char *)'.');
                    transmit_char((char *)'-');
                    transmit_char((char *)'-');
                    transmit_char((char *)'-');
                    transmit_char((char *)'-');
                    transmit_char((char *)' ');
                    
                    break;
                    case '2':

                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)'-');
                    transmit_char((char *)'-');
                    transmit_char((char *)'-');
                    transmit_char((char *)' ');

                    break;
                    case '3':

                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)'-');
                    transmit_char((char *)'-');
                    transmit_char((char *)' ');

                    break;
                    case '4':

                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)'-');
                    transmit_char((char *)' ');

                    break;                   
                    case '5':


                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)' ');

                    break;
                    case '6':

                    transmit_char((char *)'-');
                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)' ');

                    break;
                    case '7':

                    transmit_char((char *)'-');
                    transmit_char((char *)'-');
                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)' ');

                    break;    
                    case '8':

                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)'.');
                    transmit_char((char *)' ');

                    break;                   
                    case '9':
                    
                    transmit_char((char *)'-');
                    transmit_char((char *)'-');
                    transmit_char((char *)'-');
                    transmit_char((char *)'-');
                    transmit_char((char *)'.');
                    transmit_char((char *)' ');
                    break;

                    case 32:
                    
                    transmit_char((char *)' ');
                    transmit_char((char *)' ');
                    break;

                }
                        
           // printf("                                                                                           \n");
           // gotoxy(cursor_x=cursor_x+2, CURSOR_Y);
            
           // printf("%c", key);
           // fflush(stdout);
        }
        else
        {      
           
           xTaskNotify(notified_hdlr, 2UL, eSetValueWithOverwrite);
           vTaskDelete(NULL);
            
        }
       // printf("                   \n");
        
    }
    vTaskDelete(NULL);
}


static void prvTask_coder(void *pvParameters)
{
    
    char key;
    const int CURSOR_Y = (int  *)pvParameters;
    int cursor_x = 0;
    for (;;)
    {      

        if (xQueueReceive(structQueue2, &key, pdMS_TO_TICKS(400)) == pdPASS)
        {
                   
            printf("                                                                                           \n");
            gotoxy(cursor_x=cursor_x+2, CURSOR_Y);
            
            printf("%c", key);
            fflush(stdout);
        }
        else
        {      
           vTaskDelete(NULL);
            
        }
       // printf("                   \n");
        
    }
    vTaskDelete(NULL);
}

static void prvTask_coder2(void *pvParameters)
{
    
    char key;
    const int CURSOR_Y = (int  *)pvParameters;
    int cursor_x = 0;
    for (;;)
    {      

       
        if (xQueueReceive(structQueue3, &key, pdMS_TO_TICKS(10000)) == pdPASS)
        {
                   
            printf("                                                                                           \n");
            gotoxy(cursor_x=cursor_x+2, CURSOR_Y);
            
            printf("%c", key);
            fflush(stdout);
        }
        else
        {      
          printf("                                                                                           \n");
          gotoxy(0, CURSOR_Y);
          printf("");
            
        }
        
    }
    vTaskDelete(NULL);
}





void app_run(void)
{
  
    structQueue = xQueueCreate(100, // Queue length
                               1); // Queue item size

    if (structQueue == NULL)
    {
        printf("Fail on create queue\n");
        exit(1);
    }

    structQueue2 = xQueueCreate(100, // Queue length
                               1); // Queue item size

    if (structQueue2 == NULL)
    {
        printf("Fail on create queue\n");
        exit(1);
    }

    structQueue3 = xQueueCreate(100, // Queue length
                               1); // Queue item size

    if (structQueue2 == NULL)
    {
        printf("Fail on create queue\n");
        exit(1);
    }

    structQueue4 = xQueueCreate(100, // Queue length
                               1); // Queue item size

    if (structQueue2 == NULL)
    {
        printf("Fail on create queue\n");
        exit(1);
    }

    clear();
    DISABLE_CURSOR();
    printf(
        "╔═════════════════╗\n"
        "║                 ║\n"
        "╚═════════════════╝\n");

    xTaskCreate(prvTask_Notified, "Notified", configMINIMAL_STACK_SIZE, NULL, TASK2_PRIORITY, &notified_hdlr);
    xTaskCreate(prvTask_coder2, "Logger_texto", configMINIMAL_STACK_SIZE, 5, TASK4_PRIORITY, &loggerTexto_Task_hdlr);
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