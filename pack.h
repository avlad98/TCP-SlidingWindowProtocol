#include "link_emulator/lib.h"

#define T_ACK 0
#define T_NACK 1
#define T_DATA 2
#define T_FILENAME 3
#define T_FINISH 4
#define T_RECEIVED 5
#define MISSING 0
#define NOT_SENT 0
#define RECEIVED 1
#define SAVED 2
#define BUFFER_LEN (MSGSIZE - sizeof(int) * 6 - sizeof(char))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX_COUNT 10000 //10k
#define MULTIPLIER (100000000 * 0.001)

typedef struct {
    int len;
    int index;
    int checksum_len;
    int checksum_index;
    int checksum_type;
    int checksum_buffer;
    char type;
    char buffer[BUFFER_LEN];
} pack;

/* Imi calculeaza numarul de pachete necesare in care poate sa imparta fisierul */
int getNumberOfPackets(unsigned long fileLen) {
    int COUNT = (int)(fileLen / BUFFER_LEN);
    if(COUNT * BUFFER_LEN < fileLen) {
        COUNT++;
    }

    return COUNT;
}

/* Modifica zona de memorie a mesajului cu indexul specificat */
void getAckFor(int index, msg *ack) {
    pack p;
    p.index = index;
    p.type = T_ACK;

    memcpy(ack->payload, &p, sizeof(pack));
    ack->len = MSGSIZE;
}

/* Calculeaza checksum-ul folosind xor-ul pe biti discutat la curs */
int crc(char *buffer, int len) {
    unsigned char crc = 0;
    int i;
    for (i = 0; i < len; i++) {
        crc ^= buffer[i];
    }

    return crc;
}

/* Verifica integritatea mesajului pentru recv */
int crcVerify(pack *p) {
    int res = 1;

    int checksum_len = crc((char*)&p->len, sizeof(p->len));
    int checksum_index = crc((char*)&p->index, sizeof(p->index));
    int checksum_type = crc((char*)&p->type, sizeof(p->type));
    int checksum_buffer = crc((char*)p->buffer, sizeof(p->buffer));

    if(
        checksum_len != p->checksum_len ||
        checksum_index != p->checksum_index ||
        checksum_type != p->checksum_type ||
        checksum_buffer != p->checksum_buffer
    ){
        res = 0;
    }
    
    return res;
}

/* Intoarce un pointer catre o structura de mesaj alocata in care se afla
    o portiune din fisier
 */
msg *getMessageFromFile(int fd, int index) {
    pack p;
    p.index = index;
    p.len = read(fd, p.buffer, sizeof(p.buffer));
    p.type = T_DATA;
    p.checksum_buffer = crc((char*)p.buffer, sizeof(p.buffer));
    p.checksum_index = crc((char*)&p.index, sizeof(p.index));
    p.checksum_len = crc((char*)&p.len, sizeof(p.len));
    p.checksum_type = crc((char*)&p.type, sizeof(p.type));

    msg *message = (msg *)malloc(sizeof(msg));
    if(!message) return NULL;

    memcpy(message->payload, &p, sizeof(pack));
    message->len = MSGSIZE;
    return message;
}

/* Intoarce un mesaj ca mai sus doar ca pentru numele fisierului si COUNT */
msg* getFirstMessage(char *fileName, int COUNT) {
    pack fn;
    memcpy(fn.buffer, fileName, strlen(fileName) + 1);
    fn.len = COUNT;
    fn.index = 0;
    fn.type = T_FILENAME;
    fn.checksum_buffer = crc((char*)fn.buffer, sizeof(fn.buffer));
    fn.checksum_index = crc((char*)&fn.index, sizeof(fn.index));
    fn.checksum_len = crc((char*)&fn.len, sizeof(fn.len));
    fn.checksum_type = crc((char*)&fn.type, sizeof(fn.type));

    msg *first = (msg *)malloc(sizeof(msg));
    if(!first) return NULL;

    memcpy(first->payload, &fn, sizeof(pack));
    return first;
}