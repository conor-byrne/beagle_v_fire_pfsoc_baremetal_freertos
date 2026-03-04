#ifndef CUSTOM_BACNET_CONFIG_H
#define CUSTOM_BACNET_CONFIG_H

/**
 * This is the custom configuration file for the bacnet stack.
 * Defaults are in src/bacnet/config.h
 *
 * From config.h:
 *
 * @note configurations are default to values used in the example apps build.
 * Use a local copy named "bacnet-config.h" with settings configured for
 * the product specific needs for code space reductions in your device.
 * Alternately, use a compiler and linker to override these defines.
 *
 * #if defined(BACNET_CONFIG_H)
 * #include "bacnet-config.h"
 * #endif
 *
 * So, don't forget to add BACNET_CONFIG_H to your project defines!
 */

/*
 * Each client or server application must define exactly one of these
 * DataLink settings, which will control which parts of the code will be built:
 * - BACDL_ETHERNET -- for Clause 7 ISO 8802-3 ("Ethernet") LAN
 * - BACDL_ARCNET   -- for Clause 8 ARCNET LAN
 * - BACDL_MSTP     -- for Clause 9 MASTER-SLAVE/TOKEN PASSING (MS/TP) LAN
 * - BACDL_BIP      -- for ANNEX J - BACnet/IPv4
 * - BACDL_BIP6     -- for ANNEX U - BACnet/IPv6
 * - BACDL_ALL      -- Unspecified for the build, so the transport can be
 *                     chosen at runtime from among these choices.
 * - BACDL_NONE      -- Unspecified for the build for unit testing
 * - BACDL_CUSTOM    -- For externally linked datalink_xxx functions
 * - Clause 10 POINT-TO-POINT (PTP) and Clause 11 EIA/CEA-709.1 ("LonTalk") LAN
 *   are not currently supported by this project.
 */
#define BACDL_BIP

/* No BBMD for us */
#define BBMD_ENABLED        0
#define BBMD_CLIENT_ENABLED 0

/* Stick to the default */
#define MAX_APDU 1476

/* BACAPP decodes WriteProperty service requests
 * Choose the datatypes that your application supports
 */
#define BACAPP_MINIMAL
#define BACAPP_HOST_N_PORT
//#define BACAPP_NULL
//#define BACAPP_BOOLEAN
//#define BACAPP_UNSIGNED
//#define BACAPP_SIGNED
//#define BACAPP_REAL
//#define BACAPP_DOUBLE
//#define BACAPP_OCTET_STRING
//#define BACAPP_CHARACTER_STRING
//#define BACAPP_BIT_STRING
//#define BACAPP_ENUMERATED
//#define BACAPP_DATE
//#define BACAPP_TIME
//#define BACAPP_OBJECT_ID
//#define BACAPP_DATETIME

/* Set the maximum vector type sizes */
#define MAX_BITSTRING_BYTES (15)
#define MAX_CHARACTER_STRING_BYTES (MAX_APDU - 6)
#define MAX_OCTET_STRING_BYTES (MAX_APDU - 6)


/**
 * @note Control the selection of services etc to enable code size reduction
 * for those compiler suites which do not handle removing of unused functions
 * in modules so well.
 *
 * We will start with the A type services code first as these are least likely
 * to be required in embedded systems using the stack.
 */
#define BACNET_SVC_SERVER 0

#endif /* CUSTOM_BACNET_CONFIG_H */
