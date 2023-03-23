/*
Pedro Machado Peçanha - 2110535
Eric Ruas Leão - 2110694
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define TRUE 1
#define FALSE 0

typedef struct Quadro {
  int R;
  int M;
  int acesso;
  int BR;
} quadro;

typedef struct Tabela {
  int capacidade;
  int qtd;
  quadro **array;
} tabela;

void (*algoritmo)();

int tempo = 0;
int tam_quadro, tam_mem;
char rw;
int numlogs = 0;
int *listalogs;
int loop = 0;

int modifica(int endereco) {
  int tmp = tam_quadro, c = 0;
  while (tmp > 1) {
    tmp = tmp >> 1;
    c++;
  }
  return endereco >> (c + 10);
}

void LRU(tabela *vetor, int endereco) {
  unsigned int valor = -1;
  int pos = 0;
  for (int c = 0; c < vetor->capacidade; c++) {
    if (valor > vetor->array[c]->acesso) {
      pos = c;
      valor = vetor->array[c]->acesso;
    }
  }
  vetor->array[pos]->R = modifica(endereco);
  vetor->array[pos]->M = FALSE;
  vetor->array[pos]->acesso = tempo;
}

void NRU(tabela *vetor, int endereco) {

  int bitM = TRUE;
  int bitBR = TRUE;
  /*
  M - TRUE | BR - TRUE -> menos
  M - FALSE | BR - TRUE
  M - TRUE | BR - FALSE
  M - FALSE | BR - FALSE -> mais!!!
  */
  int pos = 0;
  for (int c = 0; c < vetor->capacidade; c++) {

    if (bitBR >= vetor->array[c]->BR && bitM >= vetor->array[c]->M) {
      bitBR = vetor->array[c]->BR;
      bitM = vetor->array[c]->M;
      pos = c;
    }

    if (bitM == FALSE && bitBR == FALSE) {
      break;
    }
  }
  vetor->array[pos]->R = modifica(endereco);
  vetor->array[pos]->M = FALSE;
  vetor->array[pos]->acesso = tempo;
}

void OPR(tabela *vetor, int endereco) {
  int ultimo = -1;
  int pos = 0;
  for (int i = 0; i < vetor->capacidade; i++) {
    for (int j = loop; j < numlogs; j++) {
      if (vetor->array[i]->R == modifica(listalogs[j])) {
        if (ultimo < j) {
          ultimo = j;
          pos = i;
        }
        break;
      } else if (j == (numlogs - 1)) {
        pos = i;
        vetor->array[pos]->R = modifica(endereco);
        vetor->array[pos]->M = FALSE;
        vetor->array[pos]->acesso = tempo;
        return;
      }
    }
  }
  vetor->array[pos]->R = modifica(endereco);
  vetor->array[pos]->M = FALSE;
  vetor->array[pos]->acesso = tempo;
}

void adiciona(tabela *vetor, int endereco) {
  if (vetor->qtd < vetor->capacidade) {
    vetor->array[vetor->qtd]->R = modifica(endereco);
    vetor->array[vetor->qtd]->M = FALSE;
    vetor->array[vetor->qtd]->BR = FALSE;
    vetor->qtd++;
    return;
  }
  algoritmo(vetor, endereco);
}

tabela *cria_tabela(int c) {
  tabela *vetor;
  vetor = (tabela *)malloc(sizeof(tabela));
  if (vetor == NULL) {
    perror("erro");
    exit(1);
  }
  vetor->qtd = 0;
  vetor->capacidade = c;
  vetor->array = (quadro **)malloc(sizeof(quadro *) * vetor->capacidade);
  if (vetor->array == NULL) {
    perror("erro");
    exit(1);
  }
  for (int i = 0; i < vetor->capacidade; i++) {
    vetor->array[i] = (quadro *)malloc(sizeof(quadro));
    vetor->array[i]->acesso = -1;
    vetor->array[i]->BR = FALSE;
    vetor->array[i]->M = FALSE;
    vetor->array[i]->R = FALSE;
  }
  return vetor;
}

int procura(tabela *vetor, int endereco) {
  for (int c = 0; c <= vetor->qtd && c < vetor->capacidade; c++) {
    if (vetor->array[c]->R == modifica(endereco)) {
      vetor->array[c]->acesso = tempo;
      vetor->array[c]->BR = TRUE;
      vetor->array[c]->M = FALSE;
      if (rw == 'W') {
        vetor->array[c]->M = TRUE;
      }
      return 0;
    }
  }
  return 1;
}

void libera(tabela *vetor) {
  for (int c = 0; c < vetor->capacidade; c++) {
    free(vetor->array[c]);
  }
  free(vetor->array);
  free(vetor);
}

int main(int argc, char *argv[]) {
  if (argc != 5) {
    perror("Faltam argumentos à serem passados");
    exit(1);
  }
  char *algoritmo_pag = argv[1], *log_nome = argv[2];
  if (strcmp("LRU", algoritmo_pag) && strcmp("NRU", algoritmo_pag) &&
      strcmp("OPR", algoritmo_pag)) {
    perror("Apenas os algoritmos LRU, NRU e OPR são aceitos");
    exit(1);
  }
  if (!strcmp("LRU", algoritmo_pag)) {
    algoritmo = LRU;
  } else if (!strcmp("NRU", algoritmo_pag)) {
    algoritmo = NRU;
  } else {
    algoritmo = OPR;
  }
  FILE *log = fopen(log_nome, "r");
  if (!log) {
    perror("Não foi possivel abrir o arquivo log");
    exit(1);
  }
  tam_quadro = atoi(argv[3]), tam_mem = atoi(argv[4]);
  if (tam_quadro < 8 || tam_quadro > 32) {
    perror(
        "O tamanho de cada página/quadro de página deve variar de 8 a 32 KB");
    exit(1);
  }
  if (tam_mem < 1 || tam_mem > 16) {
    perror("O tamanho total de memória física hipoteticamente disponível pode "
           "variar de 1 MB a 16 MB");
    exit(1);
  }
  int faltas = 0, escritas = 0, addr;
  int capacidade = (tam_mem * 1024) / tam_quadro;
  // int capacidade = 7;
  tabela *vetor;
  vetor = cria_tabela(capacidade);
  int a = 0;
  while (fscanf(log, "%x %c ", &addr, &rw) == 2)
    numlogs++;
  fclose(log);
  listalogs = (int *)malloc(sizeof(int) * numlogs);
  log = fopen(log_nome, "r");
  while (fscanf(log, "%x %c ", &addr, &rw) == 2) {
    listalogs[loop] = addr;
    loop++;
  }
  fclose(log);
  loop = 0;
  log = fopen(log_nome, "r");
  while (fscanf(log, "%x %c ", &addr, &rw) == 2) {
    if (rw == 'R') {
      // printf("ler\n");
      if (procura(vetor, addr)) {
        // printf("nao achou\n");
        faltas++;
        adiciona(vetor, addr);
      }
      // else
      // printf("achou\n");
    } else if (rw == 'W') {
      // printf("escrita\n");
      if (procura(vetor, addr)) {
        // printf("nao achou\n");
        faltas++;
        adiciona(vetor, addr);
        escritas++;
      }
      // else
      // printf("achou\n");
    } else {
      perror("O arquivo esta escrito de forma errada");
      exit(1);
    }
    tempo++;
    // sleep(1);
    // resetando o bit de acesso em caso de tempo
    if (!(tempo % 10)) {
      for (int i = 0; i < vetor->capacidade; i++) {
        vetor->array[i]->BR = FALSE;
      }
    }
    loop++;
  }
  libera(vetor);
  free(listalogs);
  fclose(log);
  {
    printf("Executando o simulador...\n");
    printf("Arquivo de entrada: %s\n", log_nome);
    printf("Tamanho da memoria fisica: %dMB\n", tam_mem);
    printf("Tamanho das páginas: %dKB\n", tam_quadro);
    printf("Algoritmo de substituição: %s\n", algoritmo_pag);
    printf("Número de Faltas de Páginas: %d\n", faltas);
    printf("Número de Páginas Escritas: %d\n", escritas);
  }
  return 0;
}