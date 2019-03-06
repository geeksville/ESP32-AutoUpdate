#include <ESP32-AutoUpdate.h>
#include <HTTPClient.h>
#include <MD5Builder.h>
#include <Update.h>
#include "esp_partition.h"
#include "esp_ota_ops.h"

void dump_part(const esp_partition_t *p)
{
    printf("type %d, subtype %d, address %x, label %s\n", p->type, p->subtype, p->address, p->label);

#if 0
    uint8_t sha256[32];
    esp_partition_get_sha256(p, sha256);
    printf("sha256: ");
    for (int i = 0; i < sizeof(sha256); i++)
        printf("%02x", sha256[i]);
#endif

    printf("\n");

#if 0
    esp_partition_pos_t pos;
    pos.offset = p->address;
    pos.size = p->size;
    esp_image_metadata_t meta;

    if(ESP_OK == esp_image_verify(ESP_IMAGE_VERIFY, &pos, &meta)) {
        printf("meta len %d, sha256: ", meta.image_len);
        for(int i = 0; i < sizeof(meta.image_digest); i++)
            printf("%02x", meta.image_digest[i]);

        MD5Builder md5;
        md5.begin();
        md5.add((uint8_t *) p->address, meta.image_len);
        md5.calculate();
        printf("md5: %s\n", md5.toString().c_str());
    }
#endif

    printf("\n");
}

void dump_parts(esp_partition_iterator_t i)
{
    while (i)
    {
        const esp_partition_t *p = esp_partition_get(i);
        dump_part(p);
        i = esp_partition_next(i);
    }
    // esp_partition_iterator_release(i);
}

void dump_partitions()
{
    const esp_partition_t *p = esp_ota_get_running_partition();

    if (p)
    {
        printf("running partition: ");
        dump_part(p);
    }

#if 1
    printf("app partitions:\n");
    dump_parts(esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, NULL));
    printf("data partitions:\n");
    dump_parts(esp_partition_find(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, NULL));
#endif
    /*
       running partition: type 0, subtype 16, address 10000, label app0
       sha256: 4c00131eef07a07ca3934da68f760d725f1cddf92e59616d0238b3287e106927
       app partitions:
       type 0, subtype 16, address 10000, label app0
       sha256: 4c00131eef07a07ca3934da68f760d725f1cddf92e59616d0238b3287e106927
       type 0, subtype 17, address 150000, label app1
       sha256: 4c00131eef07a07ca3934da68f760d725f1cddf9cb8d0d80101efb3f2841ff3f
       data partitions:
       type 1, subtype 2, address 9000, label nvs
       sha256: 3d99579c436304e40a033d13a37e555af8ea3a5e0a66864ebb16d90b9fbf88df
       type 1, subtype 0, address e000, label otadata
       sha256: f94c5d786a7a8fab06ac5d10e33bf37711a6697636dc037559ea19cc410a17f0
       type 1, subtype 153, address 290000, label eeprom
       sha256: f47a8ec3e9aff2318d896942282ad4fe37d6391c82914f54a5da8a37de1300c6
       type 1, subtype 130, address 291000, label spiffs
       sha256: 9bc6592295ad5b158db06e2662df7a9cb4788b3b9e0527092d7bb12760b191b4
     */
}

/**
   We look for update control files at this location (*.ucf)
 */
AutoUpdate::AutoUpdate(String baseURL) : baseURL(baseURL)
{
}

/**
   Return true if the server says an update is available
 */
bool AutoUpdate::isUpdateAvailable()
{
    // running sha256 digest https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/storage/spi_flash.html#_CPPv224esp_partition_get_sha256PK15esp_partition_tP7uint8_t
    // running image md5 via https://www.reddit.com/r/esp32/comments/9n9ujq/any_way_to_get_md5_of_currently_running_firmware/
    return false;
}

/**
   if an update is needed or force is true, do an update
 */
void AutoUpdate::update(bool force)
{
    // use streaming api to download https://github.com/espressif/arduino-esp32/blob/master/libraries/HTTPClient/examples/StreamHttpClient/StreamHttpClient.ino

    if (!force)
        return; // For the time being we only support force - FIXME

    const String fileURL = baseURL + ".bin";
    Serial.printf("Starting an update from %s\n", fileURL.c_str());
    WiFiClient client;
    HTTPClient http;
    http.begin(client, fileURL); // FIXME, we assume all msgs are urls
    int httpCode = http.GET();
    Serial.printf("HTTP get %d\n", httpCode);
    if (httpCode == HTTP_CODE_OK && http.getSize() > 0)
    {
        // String payload = http.getString(); // This string can be enormous
        int len = http.getSize();

        Serial.printf("got update header %d\n", len);

        // Check if there is enough to OTA Update
        bool canBegin = Update.begin(len);

        // If yes, begin
        if (canBegin)
        {
            Serial.println("Begin OTA. This may take a minute to complete, please wait.");
            // No activity would appear on the Serial monitor
            // So be patient. This may take 2 - 5mins to complete
            size_t written = Update.writeStream(http.getStream());

            if (written == len)
            {
                Serial.println("Written successfully");
            }
            else
            {
                Serial.println("Written only : " + String(written) + "/" + String(len));
                // retry??
                // execOTA();
            }

            if (Update.end())
            {
                Serial.println("OTA done!");
                if (Update.isFinished())
                {
                    Serial.println("Update successfully completed. Rebooting.");
                    ESP.restart();
                }
                else
                {
                    Serial.println("Update not finished? Something went wrong!");
                }
            }
            else
            {
                Serial.println("Error Occurred. Error #: " + String(Update.getError()));
            }
        }
        else
        {
            Serial.println("Not enough space to begin OTA");
        }
    }
    http.end(); // free buffers
}

/**
   Something is really wrong with this appload, force a rollback to the previous version (if possible).
   If this routine succeeds it will not return
 */
void AutoUpdate::doRollback()
{
}

/** Tell the upgrader that the currently running appload is good in all important ways and further failures should not trigger a rollback
 */
void AutoUpdate::declareSuccess()
{
}

#if 0
openssl genrsa -out private.key 2048
openssl rsa -in private.key -outform PEM -pubout -out public.key

<in globals>
BearSSL::PublicKey signPubKey( ... key contents ... );
BearSSL::HashSHA256 hash;
BearSSL::SigningVerifier sign( &signPubKey );
...
<in setup()>
Update.installSignature( &hash, &sign );
#endif
