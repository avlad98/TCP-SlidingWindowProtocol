#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "link_emulator/lib.h"
#include "pack.h"

#define HOST "127.0.0.1"
#define PORT 10001

int main(int argc,char** argv){
  init(HOST,PORT);

  int fd = -1;
  int COUNT = MAX_COUNT;
  char newName[50] = "recv_";

  msg m;
  msg ack;

  /* Aloc vector pentru a salva mesajele primite */
  msg *messages = (msg *)calloc(MAX_COUNT, sizeof(msg));
  if(messages == NULL) {printf("[RECV] Eroare la alocare vector de mesaje\n"); return -1;}

  int counter = 0;
  int lastWritten = 1;
  int fileIsOpened = 0;

  /* Astept mereu din retea mesaje */
  while (1)
  {
    recv_message(&m);
    pack *p = (pack *)m.payload;

    /* Verific CHECKSUM */
    if(crcVerify(p)){
      /* Trimit ACK */
      getAckFor(p->index, &ack);
      send_message(&ack);

      /* Stochez mesajul primit daca nu exista */
      if ((messages + p->index)->len != RECEIVED)
      {
        msg *mptr = messages + p->index;
        memcpy(mptr, &m, sizeof(msg));
        mptr->len = RECEIVED;
        counter++;

        /* Verific daca am primit primul mesaj cu numele fisierului si COUNT-ul
            si deschid fisierul 
        */
        if (p->index == 0)
        {
          COUNT = p->len;
          strcat(newName, p->buffer);

          fd = open(newName, O_CREAT | O_WRONLY, 0644);
          if(fd < 0) {printf("[RECV] Eroare la deschidere fisier %s\n", newName); return -1;}
          fileIsOpened = 1;
        }

        /* Daca am primit toate pachetele anunt SEND sa se opreasca din trimis */
        if (counter == COUNT) {
          pack recvd;
          recvd.type = T_RECEIVED;
          memcpy(ack.payload, &recvd, sizeof(pack));
          ack.len = MSGSIZE;
          send_message(&ack);
          break;
        }

        /* Daca fisierul este disponibil scriu in el la rand mesajele cat se poate */
        if(fileIsOpened){
          while((messages + lastWritten)->len == RECEIVED) {
              write(fd, ((pack *)(messages + lastWritten)->payload)->buffer, ((pack *)(messages + lastWritten)->payload)->len);
              lastWritten++;
          }
        }
          
      }
      else {
        /* Pachetul exista deja, il arunc pe jos */
      }
    }else{
      /* Am primit mesaj corupt, il arunc pe jos */
      continue;
    }
  }

  /* Termin de scris restul mesajelor in fisier */
  while(lastWritten != COUNT) {
    write(fd, ((pack *)(messages + lastWritten)->payload)->buffer, ((pack *)(messages + lastWritten)->payload)->len);
    lastWritten++;
  }

  /* Eliberez memoria */
  free(messages);
  close(fd);

  /* Trimit lui SEND confirmare ca am terminat de scris in fisier */
  pack finish;
  finish.type = T_FINISH;
  memcpy(ack.payload, &finish, sizeof(pack));
  ack.len = MSGSIZE;
  send_message(&ack);

  return 0;
}