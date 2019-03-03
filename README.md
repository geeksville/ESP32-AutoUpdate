# ESP32-AutoUpdate

Making secure software updates from the cloud for your ESP32 project/product.

WARNING: This libary is still in alpha testing.  You probably don't want it yet.

Notes to self:
* fetch URL.ucf a small file that contains the following, one per line:
** integer version # (optional - to check against current appload version #)
** version string (optional to show users)
** sha256 signature of the load (32 bytes, printed as hex digits)
** url of the bin to load (or if empty string assumed to be URL.bin)

* usually URL.bin has the bin that goes with that UCF file
