#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>

#define MAX_PROGRAMS 5
#define MAX_TIME 3600 // Tempo máximo de 1 hora

typedef struct {
    char name[20];
    int time; // em segundos
    int power_level;
} Program;

Program programs[MAX_PROGRAMS] = {
    {"Carnes", 300, 8},
    {"Peixe", 200, 6},
    {"Frango", 250, 7},
    {"Lasanha", 400, 9},
    {"Pipoca", 150, 10}
};

// Estado do micro-ondas
bool is_heating = false;   // Indica se está aquecendo
bool door_open = false;    // Indica se a porta está aberta
pthread_t heating_thread;  // Thread para simular o aquecimento
bool cancel_process = false;

// Protótipos de funções
void display_menu();
void display_clock();
void choose_program();
void set_manual_time();
void emergency_stop();
void program_future_action();
void simulate_heating(const char *program_name, int time, int power);
void *heating_cycle(void *args);
void light_control(bool on);
void beep();
void rotate_plate();
void cooling_process();
void refresh_screen();
void clear_screen();

// Estrutura para passar os argumentos à thread
typedef struct {
    char name[20];
    int time;
    int power;
} HeatingArgs;

// Exibe o menu principal
void display_menu() {
    printf("\n\n=== Microondas Simulado ===\n");
    printf("1. Escolher Programa\n");
    printf("2. Ajustar Tempo Manualmente\n");
    printf("3. Emergencia: Cancelar e Abrir Porta\n");
    printf("4. Programar Ligacao Automatica\n");
    printf("5. Sair\n");
}

// Exibe o relógio cortesia
void display_clock() {
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    printf("\nHora atual: %02d:%02d:%02d\n", local->tm_hour, local->tm_min, local->tm_sec);
}

// Simula o bip do micro-ondas
void beep() {
    printf("\a"); // Som de bip
    fflush(stdout);
}

// Simula o controle da luz interna
void light_control(bool on) {
    if (on) {
        printf(">> Luz interna: Ligada.\n");
    } else {
        printf(">> Luz interna: Desligada.\n");
    }
}

// Simula a rotação do prato
void rotate_plate() {
    printf(">> Prato girando...\n");
}

// Simula o resfriamento do micro-ondas
void cooling_process() {
    printf(">> Resfriando o micro-ondas...\n");
    sleep(5); // Simula o tempo de resfriamento
    printf(">> Micro-ondas resfriado.\n");
}

// Simula o aquecimento
void *heating_cycle(void *args) {
    HeatingArgs *heating_args = (HeatingArgs *)args;
    const char *program_name = heating_args->name;
    int time = heating_args->time;
    int power = heating_args->power;

    light_control(true); // Liga a luz
    printf(">> Iniciando programa '%s': %d segundos, potencia %d.\n", program_name, time, power);

    for (int remaining_time = time; remaining_time > 0; remaining_time--) {
        if (cancel_process || door_open) {
            printf(">> Processo cancelado ou porta aberta!\n");
            light_control(false); // Desliga a luz
            pthread_exit(NULL);
        }
        printf("Tempo restante: %d segundos\n", remaining_time);
        sleep(1); // Simula 1 segundo de aquecimento
    }

    light_control(false); // Desliga a luz
    printf(">> Programa '%s' finalizado!\n", program_name);
    beep();
    cooling_process();
    free(args); // Libera a memória alocada para os argumentos
    refresh_screen();
    pthread_exit(NULL);
}

void simulate_heating(const char *program_name, int time, int power) {
    HeatingArgs *args = (HeatingArgs *)malloc(sizeof(HeatingArgs));
    strcpy(args->name, program_name);
    args->time = time;
    args->power = power;

    cancel_process = false; // Reseta o estado de cancelamento
    pthread_create(&heating_thread, NULL, heating_cycle, (void *)args);
}

void choose_program() {
    printf("\n** Programas Disponiveis **\n");
    for (int i = 0; i < MAX_PROGRAMS; i++) {
        printf("%d. %s - Tempo: %d segundos - Potencia: %d\n",
               i + 1, programs[i].name, programs[i].time, programs[i].power_level);
    }

    int choice;
    printf("Escolha um programa: ");
    scanf("%d", &choice);

    if (choice >= 1 && choice <= MAX_PROGRAMS) {
        Program selected_program = programs[choice - 1];
        simulate_heating(selected_program.name, selected_program.time, selected_program.power_level);
    } else {
        printf("Opcao invalida.\n");
    }
}

void set_manual_time() {
    int time, power;
    printf("Digite o tempo em segundos: ");
    scanf("%d", &time);
    printf("Digite o nivel de potencia (1-10): ");
    scanf("%d", &power);

    if (time > 0 && time <= MAX_TIME && power >= 1 && power <= 10) {
        simulate_heating("Manual", time, power);
    } else {
        printf("Configuracao invalida.\n");
    }
}

void emergency_stop() {
    cancel_process = true; // Marca o processo como cancelado
    pthread_cancel(heating_thread); // Cancela a thread
    printf(">> Processo cancelado.\n");
    light_control(false);
}

void program_future_action() {
    int delay;
    printf("Digite o atraso em segundos para iniciar o ciclo: ");
    scanf("%d", &delay);

    if (delay > 0) {
        int choice;
        printf("\n** Programas Disponiveis **\n");
        for (int i = 0; i < MAX_PROGRAMS; i++) {
            printf("%d. %s - Tempo: %d segundos - Potencia: %d\n",
                   i + 1, programs[i].name, programs[i].time, programs[i].power_level);
        }

        printf("Escolha um programa: ");
        scanf("%d", &choice);

        if (choice >= 1 && choice <= MAX_PROGRAMS) {
            Program selected_program = programs[choice - 1];
            printf("Aguardando %d segundos para iniciar o programa '%s'...\n", delay, selected_program.name);
            sleep(delay); // Espera pelo atraso configurado
            simulate_heating(selected_program.name, selected_program.time, selected_program.power_level);
        } else {
            printf("Opcao invalida.\n");
        }
    } else {
        printf("Atraso invalido.\n");
    }
}

void clear_screen() {
#ifdef _WIN32
    system("cls"); // Limpa a tela no Windows
#else
    printf("\033[H\033[J"); // Limpa a tela em sistemas Unix
#endif
}

void refresh_screen() {
    display_clock();
    display_menu();
}

int main() {
    int choice;

    while (1) {
        refresh_screen();
        printf("Escolha uma opcao: ");
        scanf("%d", &choice);

        switch (choice) {
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
                program_future_action();
                break;
            case 5:
                printf("Saindo...\n");
                exit(0);
            default:
                printf("Opcao invalida.\n");
        }
        refresh_screen();
    }

    return 0;
}
