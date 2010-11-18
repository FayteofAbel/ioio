//
// USB Host Android Driver (Header File)
//
// This is the Android driver file for a USB Embedded Host device.  This
// driver should be used in a project with usb_host.c to provide the USB
// hardware interface.
//
// To interface with USB Embedded Host layer, the routine USBHostAndroidInit()
// should be specified as the Initialize() function, and
// USBHostAndroidEventHandler() should be specified as the EventHandler()
// function in the usbClientDrvTable[] array declared in usb_config.c.
//
// The driver passes on the following events to the application event handler
// (defined by the USB_HOST_APP_EVENT_HANDLER macro):
// - EVENT_DETACH
// - EVENT_SUSPEND
// - EVENT_RESUME
// - EVENT_BUS_ERROR
//
// This driver can be configured to either use transfer events from usb_host.c
// or use a polling mechanism.  If USB_ENABLE_TRANSFER_EVENT is defined, this
// driver will utilize transfer events.  Otherwise, this driver will utilize
// polling.
//
// Since the generic class is performed with interrupt transfers,
// USB_SUPPORT_INTERRUPT_TRANSFERS must be defined.

#ifndef __USBHOSTANDROID_H__
#define __USBHOSTANDROID_H__

////////////////////////////////////////////////////////////////////////////////
// The following code is not intended for the client to use directly.
////////////////////////////////////////////////////////////////////////////////

// Generic Device ID Information
// This structure contains identification information about an attached device.
typedef struct _ANDROID_DEVICE_ID {
  WORD  vid;            // Vendor ID of the device
  WORD  pid;            // Product ID of the device
  BYTE  deviceAddress;  // Address of the device on the USB
} ANDROID_DEVICE_ID;

// Generic Device Information
// This structure contains information about an attached device, including
// status flags and device identification.
typedef struct _ANDROID_DEVICE {
  ANDROID_DEVICE_ID ID;             // Identification information about the device
  DWORD             rxLength;       // Number of bytes received in the last IN transfer
  BYTE              clientDriverID; // ID to send when issuing a Device Request
  BYTE              inEndpoint;     // Address of endpoint from which we read
  BYTE              outEndpoint;    // Address of endpoint to which we write
  BYTE              rxErrorCode;    // Error code of last IN transfer
  BYTE              txErrorCode;    // Error code of last OUT transfer

  union {
    BYTE val;                       // BYTE representation of device status flags
    struct {
      BYTE initialized    : 1;      // Driver has been initialized
      BYTE txBusy         : 1;      // Driver busy transmitting data
      BYTE rxBusy         : 1;      // Driver busy receiving data
    };
  } flags;                          // Android driver status flags
} ANDROID_DEVICE;

extern ANDROID_DEVICE   gc_DevData; // Information about the attached device.

////////////////////////////////////////////////////////////////////////////////
// The following two functions are to be put in the driver table and called by
// the USB host layer. Should not be called directly by the client.
////////////////////////////////////////////////////////////////////////////////

BOOL USBHostAndroidInit ( BYTE address, DWORD flags, BYTE clientDriverID );
BOOL USBHostAndroidEventHandler ( BYTE address, USB_EVENT event, void *data, DWORD size );


////////////////////////////////////////////////////////////////////////////////
// The following code is the public API of the driver - should be used by the
// client.
////////////////////////////////////////////////////////////////////////////////

// Check whether a device is currently attached.
#define USBHostAndroidIsDeviceAttached() ( (gc_DevData.flags.initialized == 1) ? TRUE : FALSE )

// Returns the ID of the currently attached device.
// Device must be attached at the time of call, and argument must not be NULL.
void USBHostAndroidGetDeviceId(ANDROID_DEVICE_ID *pDevID);

// Issue a read request from the device.
// Actual read will be done asynchronously. Client should call
// USBHostAndroidRxIsComplete() to check for completion and get status code.
// Returns USB_SUCCESS if succeeded.
// Device must be attached.
BYTE USBHostAndroidRead(void *buffer, DWORD length);

// Check whether the last call to USBHostAndroidRead has completed.
// In case it is complete, returns TRUE, and the error code and number of bytes
// read are returned.
BOOL USBHostAndroidRxIsComplete(BYTE *errorCode, DWORD *byteCount);

// This function must be called periodically by the client to provide context to
// the driver IF NOT working with transfer events (USB_ENABLE_TRANSFER_EVENT)
// It will poll for the status of transfers.
#ifndef USB_ENABLE_TRANSFER_EVENT
void USBHostAndroidTasks( void );
#endif  // USB_ENABLE_TRANSFER_EVENT

// Issue a read request to the device.
// Actual write will be done asynchronously. Client should call
// USBHostAndroidTxIsComplete() to check for completion and get status code.
// Returns USB_SUCCESS if succeeded.
// Device must be attached.
BYTE USBHostAndroidWrite(void *buffer, DWORD length);

// Check whether the last call to USBHostAndroidWrite has completed.
// In case it is complete, returns TRUE, and the error code is returned.
BOOL USBHostAndroidTxIsComplete(BYTE *errorCode );


#endif  // __USBHOSTANDROID_H__
