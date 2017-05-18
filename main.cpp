
#include "mbed.h"
#include "AUDIO_GRBoard.h"

#define WRITE_BUFF_NUM         (16)
#define READ_BUFF_SIZE         (4096)
#define READ_BUFF_NUM          (16)
#define MAIL_QUEUE_SIZE        (WRITE_BUFF_NUM + READ_BUFF_NUM + 1)
#define INFO_TYPE_WRITE_END    (0)
#define INFO_TYPE_READ_END     (1)

AUDIO_GRBoard audio(0x80, WRITE_BUFF_NUM, READ_BUFF_NUM);

typedef struct {
    uint32_t info_type;
    void *   p_data;
    int32_t  result;
} mail_t;
Mail<mail_t, MAIL_QUEUE_SIZE> mail_box;

//32 bytes aligned! No cache memory
#if defined(__ICCARM__)
#pragma data_alignment=32
static uint8_t audio_read_buff[READ_BUFF_NUM][READ_BUFF_SIZE]@ ".mirrorram";
#else
static uint8_t audio_read_buff[READ_BUFF_NUM][READ_BUFF_SIZE] __attribute((section("NC_BSS"),aligned(32)));
#endif

static void callback_audio(void * p_data, int32_t result, void * p_app_data) {
    mail_t *mail = mail_box.alloc();

    if (result < 0) {
        printf("error %ld\r\n", result);
    }
    if (mail == NULL) {
        printf("error mail alloc\r\n");
    } else {
        mail->info_type = (uint32_t)p_app_data;
        mail->p_data    = p_data;
        mail->result    = result;
        mail_box.put(mail);
    }
}

int main() {
    rbsp_data_conf_t audio_write_conf = {&callback_audio, (void *)INFO_TYPE_WRITE_END};
    rbsp_data_conf_t audio_read_conf  = {&callback_audio, (void *)INFO_TYPE_READ_END};

    audio.power();
    audio.outputVolume(1.0, 1.0);
    audio.micVolume(0.7);

    // Read buffer setting
    for (int i = 0; i < READ_BUFF_NUM; i++) {
        if (audio.read(audio_read_buff[i], READ_BUFF_SIZE, &audio_read_conf) < 0) {
            printf("read error\r\n");
        }
    }

    while (1) {
        osEvent evt = mail_box.get();
        if (evt.status == osEventMail) {
            mail_t *mail = (mail_t *)evt.value.p;

            if ((mail->info_type == INFO_TYPE_READ_END) && (mail->result > 0)) {
                audio.write(mail->p_data, mail->result, &audio_write_conf);
            } else {
                audio.read(mail->p_data, READ_BUFF_SIZE, &audio_read_conf); // Resetting read buffer
            }
            mail_box.free(mail);
        }
    }
}

