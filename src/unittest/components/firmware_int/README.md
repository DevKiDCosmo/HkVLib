Firmware Integrety test

Checks sha256 sums. Cmake change. create makro sha256-sum for verification

DEPRECATED through Esp-idf boot

```cpp
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_system.h"

void print_running_firmware_sha256(void)
{
    const esp_partition_t *running = esp_ota_get_running_partition();

    uint8_t sha_256[32];
    esp_partition_get_sha256(running, sha_256);

    printf("Running firmware SHA256: ");
    for (int i = 0; i < 32; i++) {
        printf("%02x", sha_256[i]);
    }
    printf("\n");
}
```