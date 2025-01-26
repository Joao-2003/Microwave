/* main.c - Micro-ondas com Xinu AVR */

#include <xinu.h>

#define MAX_PROGRAMS     5
#define MAX_TIME         3600    // 1 hora em segundos
#define HEATING_STACK    2048    // Tamanho da pilha para processo de aquecimento
#define HEATING_PRIO     20      // Prioridade do processo


/* Estruturas de dados */
typedef struct {
    char name[20];
    int time;        // Tempo em segundos
    int power;       // Nível de potência (1-10)
} Program;

typedef struct {
    char name[20];
    int time;
    int power;
} HeatingArgs;

/* Programas pré-definidos */
const Program programs[MAX_PROGRAMS] = {
    {"Carnes", 300, 8},
    {"Peixe", 200, 6},
    {"Frango", 250, 7},
    {"Lasanha", 400, 9},
    {"Pipoca", 150, 10}
};

/* Variáveis globais e semáforos */
volatile pid32 heating_pid = -1;
volatile bool8 cancel_flag = FALSE;
sid32 cancel_sem;

/* Protótipos de funções */
void display_menu(void);
void choose_program(void);
void set_manual_time(void);
void emergency_stop(void);
void heating_process(int32 args, int32 dummy);
void light_control(bool8 on);
void rotate_plate(void);
void cooling(void);
void beep(void);

/* Controle da luz interna */
void light_control(bool8 on) {
    kprintf(on ? "\n>> Luz interna: LIGADA" : "\n>> Luz interna: DESLIGADA");
}

/* Rotação do prato */
void rotate_plate(void) {
    kprintf("\n>> Prato girando...");
}

/* Sistema de resfriamento */
void cooling(void) {
    kprintf("\n>> Resfriando...");
    sleep(5);
    kprintf("\n>> Pronto para uso!");
}

/* Sinal sonoro */
void beep(void) {
    kprintf("\a"); // Caractere especial para beep
}

/* Processo de aquecimento */
void heating_process(int32 args, int32 dummy) {
    HeatingArgs *params = (HeatingArgs *)args;
    int i;
    
    light_control(TRUE);
    kprintf("\n>> INICIANDO: %s", params->name);
    kprintf("\n>> Tempo: %ds | Potencia: %d", params->time, params->power);

    for(i = params->time; i > 0; i--) {
        if (cancel_flag) {
            kprintf("\n>> PROCESSO CANCELADO!");
            break;
        }
        
        rotate_plate();
        kprintf("\n>> Tempo restante: %d segundos", i);
        sleep(1);
    }

    light_control(FALSE);
    beep();
    cooling();
    
    // Liberar memória e resetar estado
    freemem((void *)params, sizeof(HeatingArgs));
    heating_pid = -1;
}

/* Menu principal */
void display_menu(void) {
    kprintf("\n\n=== MICROONDAS Xinu ===");
    kprintf("\n1 - Programas pre-definidos");
    kprintf("\n2 - Tempo manual");
    kprintf("\n3 - Cancelar operacao");
    kprintf("\n4 - Sair");
    kprintf("\n========================");
    kprintf("\nSelecione: ");
}

/* Seleção de programa */
void choose_program(void) {
    char c;
    int choice;
    
    kprintf("\n\n=== PROGRAMAS ===");
    for(int i=0; i<MAX_PROGRAMS; i++) {
        kprintf("\n%d - %s (%ds @ %dW)", 
            i+1, programs[i].name, programs[i].time, programs[i].power);
    }
    kprintf("\nEscolha: ");
    
    c = getchar();
    choice = c - '0' - 1;

    if(choice >= 0 && choice < MAX_PROGRAMS) {
        HeatingArgs *args = (HeatingArgs *)getmem(sizeof(HeatingArgs));
        strlcpy(args->name, programs[choice].name, 20);
        args->time = programs[choice].time;
        args->power = programs[choice].power;
        
        heating_pid = create(heating_process, HEATING_STACK, 
                           HEATING_PRIO, "aquecimento", 2, (int32)args, 0);
        if(heating_pid != -1) {
            resume(heating_pid);
        }
    }
}

/* Configuração manual */
void set_manual_time(void) {
    char buf[10];
    HeatingArgs *args = (HeatingArgs *)getmem(sizeof(HeatingArgs));
    
    kprintf("\nTempo (segundos): ");
    read(CONSOLE, buf, 10);
    args->time = atoi(buf);
    
    kprintf("\nPotencia (1-10): ");
    read(CONSOLE, buf, 10);
    args->power = atoi(buf);
    
    strlcpy(args->name, "Manual", 20);
    
    heating_pid = create(heating_process, HEATING_STACK, 
                       HEATING_PRIO, "aquecimento", 2, (int32)args, 0);
    if(heating_pid != -1) {
        resume(heating_pid);
    }
}

/* Parada de emergência */
void emergency_stop(void) {
    if(heating_pid != -1) {
        cancel_flag = TRUE;
        kill(heating_pid);
        heating_pid = -1;
        kprintf("\n>> OPERACAO CANCELADA!");
    }
}

/* Processo principal */
process main(void) {
    char c;
    int choice;
    
    // Inicialização do sistema
    cancel_sem = semcreate(1);
    kprintf("\n\nSistema Microondas Xinu Iniciado");

    while(TRUE) {
        display_menu();
        
        c = getchar();
        choice = c - '0';
        
        switch(choice) {
            case 1:
                choose_program();
                break;
                
            case 2:
                set_manual_time();
                break;
                
            case 3:
                emergency_stop();
                break;
                
            case 4:
                kprintf("\nDesligando...");
                return OK;
                
            default:
                kprintf("\nOpcao invalida!");
        }
    }
    return OK;
}
