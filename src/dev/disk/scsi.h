//
//  scsi.h
//  A2Mac
//
//  Created by Tamas Rudnai on 7/7/22.
//  Copyright © 2022 GameAlloy. All rights reserved.
//

#ifndef scsi_h
#define scsi_h

#include <stdio.h>



/// Making a Smartport call
/// The Operating System or program running on the Apple II executes a call to the Smartport in three basic stages, as follows:
///     • Smartport location
///     • Command parameters Zero-Page write
///     • Smartport call

/// Smartport call
/// The fInal stage is the actual Smartport call. A Smartport call is coded as a JSR to the Smartport entry point (DISPATCH),
/// followed by the one-byte command number, followed by the tv,lo-byte command parameter list pointer.
/// You can calculate the DISPATCH address, as follows:
///
/// $Cnoo + (CnFF ) + 3
///
/// where n is the SCSI Card slot number and CnFF is the value of the byte located at address CnFF.
///
/// An example of a Smartport call is as follows:

// PCCALL   JSR DISPATCH            ; Call SMP entry point and ; dispatcher
//          DFB CMDNUM              ; SMP command number
//          DW  CMDLlST             ; Command Parameter list pointer
//          BCS ERROR               ; Carry is set on an error

/// When the Smartport has completed the operation requested by the call,
/// it sets certain flags in the microprocessor's Status register and accumulator (A register).
/// The state of these flags depends on whether or not the Smartport call was successfully completed
/// Table 3-1 defInes the state of the microprocessor flags for both successful and unsuccessful Smartport calls.

// Table 3-1, Microporcessor Register State
//
// Bit      Successful Call     Unsuccessful Call
// N        x*                  x
// Z        x                   x
// C        0                   1
// D        0                   0
// V        x                   x
// 1        unchanged           unchanged
// B        x                   x
// Xreg     x                   x
// Yreg     x                   x
// Acc      0                   error
// SMP      JSR+3               JSR+3
// S        unchanged           unchanged

// *x is undefined unless index information is returned in X and Y.

/// To locate the Smartport, search for the following bytes:
///     • $Cn0l = $20 (where n is the SCSI Card slot number)
///     • $Cn03 = $00
///     • $Cn05 = $03
///     • $Cn07 = $00 (this is the address used by the SCSI Card)


typedef struct smartport_s {
    
} smartport_t;


typedef enum : uint8_t {
    scsi_cmd = 0x42,
    scsi_unit = 0x43,
    scsi_buf_ptr = 0x44,
    scsi_blck_num = 0x46
} scsi_cmd_param_zp_t;


typedef struct {
    uint8_t scsi_block_device   : 1;
    uint8_t scsi_write_allowed  : 1;
    uint8_t scsi_read_allowed   : 1;
    uint8_t scsi_device_online  : 1;
    uint8_t scsi_format_allowed : 1;
    uint8_t scsi_write_protected: 1;
    uint8_t scsi_interrupt      : 1;
    uint8_t scsi_device_open    : 1;
} scsi_status_byte_t;


typedef enum : uint8_t {
    scsi_unit_number_smartport = 0x00,
/// Unit Number $00: A Code $00 STATIJS command with the Unit Number set to $00 returns the status of the Smartport itself.
/// The status list returned is 8 bytes long. Table 3-3 defInes the content of the Smartport status list
} scsi_unit_number_t;


typedef struct {
    uint8_t scsi_smprt_number_of_devices;
    uint8_t scsi_smprt_interrupt_state;
/// Bit 7 of Byte 1 is used to indicate that a device on the SCSI bus has generated an interrupt
/// The status list does not indicate the specific device that generated the interrupt;
/// the Apple II must poll each of the devices on the SCSI bus to determine which device requires interrupt handling.
    uint8_t scsi_smprt_reserved[5];
} scsi_smartport_status_t;


typedef enum : uint8_t {
    scsi_code_device_status = 0x00,
/// Code $00: Device Status
///     The status list returned for this code is four bytes long.
///     The fIrst byte is the general status byte, and the remaining three bytes are the size of the device in 512-byte blocks.
///     The general status byte is returned for all device types, but the size bytes are only returned for block-type devices.
///     Table 3-2 defInes the information returned in the general status byte.
///     See: scsi_status_byte_t

    
    scsi_code_device_control_block = 0x01,
/// Code$01: Device Control Block
///     The status list returned for this code ranges from 1 to 256 bytes long, depending on the content
///     of the Device Control Block (DeB). The first byte of the DeB contains the number of bytes in the block.
///     The information returned in this status list is device-dependent.


    scsi_code_newline_status = 0x02,
/// Code$02: Newline Status (character devices only)
///     The status list returned for this code has not been defined.

    
    scsi_code_device_info_block = 0x03,
/// Code $03: Device Information Block
///     The status list returned for this code is 23 bytes long. The flrst four bytes are identical to the bytes returned for Code $00.
///     The remaining bytes are device-specific status information. Table 3-4 defmes the information returned in the device-specific status bytes.
} scsi_status_code_t;


typedef enum : uint8_t {
    scsi_err_BUSERR     = 0x06,             // SCSI bus erro
    scsi_err_BADCTL     = 0x21,             // Illegal Status
    scsi_err_BADCTLPARM = 0x22,             // Invalid parameter list
    scsi_err_IOERROR    = 0x27,             // I/O Error
    scsi_err_NODRIVE    = 0x28,             // Target device not present on SCSI bus
    scsi_err_NOWRITE    = 0x2B,             // Device / Media write protected
    scsi_err_BADBLOCK   = 0x2D,             // Illegal Block Number
    scsi_err_OFFLINE    = 0x2F,             // Device not online, no disk in drive

    // $30-$3F                              // Device-specific error
    // $50-$7F                              // Device-specific error
} scsi_err_codes_t;


typedef struct {
    uint8_t     scsi_dev_status;            // General Status Byte
    uint32_t    scsi_dev_number_of_blocks : 24; // 3 bytes
    uint32_t    scsi_dev_name_len : 8;      // max 16
    uint8_t     scsi_dev_name[16];
    uint8_t     scsi_dev_type;              // Device type mnemonic
    uint8_t     scsi_dev_subtype;           // Device subtype mnemonic
    uint8_t     scsi_dev_firmware_version;  // Device type mnemonic
} scsi_device_status_t;


typedef enum : uint8_t {
    scsi_ctrl_comm_device_reset         = 0x00,     // Device Reset
/// Code $00: This command orders a soft reset of the target device.
///     The control list contains device-specific information required by the device controller to reinitialize itself.
    
    scsi_ctrl_comm_set_dcb              = 0x01,     // Set device Control Block (DCB)
/// Code $01: This command writes the DCB. The DCB is typically used to control the operating parameters of the device itself.
///     Since the length of the DCB is device-dependent, Apple recommends the following procedure for altering the DCB:
///         1) Read in the DCB by using the STATUS $01 command
///         2) Make the desired changes
///         3) Write the modified DCB using this control command

    scsi_ctrl_comm_set_newline_status   = 0x02,     // Set newline status (character device only)
/// Code $02: NOT DEFINED
    
    scsi_ctrl_comm_handle_dev_interrupt = 0x03,     // Handle device interrupt
/// Code $03: NOT DEFINED

    
/// Unit Number $00 Commands:
///     A CONTROL command issued with a Unit Number of $00 affects all devices on the SCSI bus.
///     This type of CONTROL command uses a special set of control commands.
    
    scsi_ctrl_comm_smprt_enable_int     = 0x00,     // Enable Interrupt Handling for Smartport
/// Code $00: This command enables interrupt handling for the Smartport.
    
    scsi_ctrl_comm_smprt_disable_int    = 0x01,     // Disable Interrupt Handling for Smartport
/// Code $01: This command disables interrupt handling for the Smartport.
    
} scsi_control_command_t;


typedef enum : uint8_t {
    scsi_cmd_status = 0x00,
/// STATUS ($00)
///    The STATUS command returns information about a specific device on the SCSI bus.
///    The information returned by this command includes the following:
///    • General status information
///        • Type of device (character or block)
///        • Type of protection (ReadIWrite), if any
///        • Format or formats allowed on the device
///        • Device state (on or off-line)
///    • Device-speciflc status information
///        • Number of logical blocks residing on the device
///        • Device name (in ASCII)
///        • Device type
///        • Device subtype
///        • Device revision number
///
///    The device type and device subtype fields d~cribe the device's physical type, such as "tape drive". etc...
///    The device type and subtype is described using a mnemontc
///
/// Description
///    The STATUS command returns infonnation about a specific device on the SCSI bus.
///    The information returned by this command is determined by the status cqde parameter.
///    On return from a STATUS call, the microprocessor X and Y registers are set to indicate
///    the number of bytes transfered to the Apple II by the command.
///    The Xregister is set to the low byte of the count, and the Y register is set to the high byte.
///
/// Parameter list
///    • Parameter Count (0): 1 byte, set to 3 for this command
///    • Unit Number (1): 1 byte, in the range $00 - $7E
///    • Status List Pointer (2,3)*: 1 word, data buffer start address • Status Code (4): 1 byte, in the range $00 - $FF
///    * These two bytes are processed as a single, one-word (1-bit) parameter.
///
/// Parameter description
///    Unit Number: This parameter contains the unit number of the target SCSI device,
///    as loaded into the Zero-Page address $43. If the Unit Number is set to $00,
///    all devices resident on the SCSI bus are targeted.
///
///    Status List Polnters: This parameter contains the beginning address of the
///    data buffer used to store the device status information returned by this command.
///    The first byte (byte 2) is the low byte, and the second byte (byte 3) is the high byte of the address.
///
///    Status Code: This parameter contains the hexadecimal code number indicating
///    which status request is being issued, as follows:
///
///    • $00    Device Status
///    • $01    Device Control Block
///    • $02    Newline Status (character devices only)
///    • $03    Device Information Block

    
    scsi_cmd_read_block = 0x01,
/// READ BLOCK ($01)
///    The READ BLOCK command reads a 512-byte block from a device.
/// Description
///    The READ BLOCK command reads one 512-byte block from the target device specified in the Unit Number parameter.
///    The block read by this command is written into bank $00 RAM at the address specified in the data buffer pointer.
/// Parameter list
///    • Parameter Count (0): 1 byte, set to 3 for this command
///    • Unit Number (1): 1 byte, in the range $00 - $7E
///    • Data Buffer Pointer (2,3)*: 1 word, data buffer address
///    • Block Number (4,5,~: 3 bytes, logical address of target block
/// *These bytes are processed as a single parameter.
///
/// Parameter description
///    • Unit Numbor: This parameter contains the unit number of the target SCSI device, as loaded into the Zero-Page address $43.
///    • Data Buffer Polnter: This parameter contains the beginning address of the data buffer used to store the block read by this command.
///         The first byte (byte 2) is the low byte, and the second byte (byte 3) is the high byte of the address.
///    • Block Number: This parameter contains the logical address, on the host device, of the target block.
///         Byte 4 is the low byte of this address, byte 5 the middle byte, and byte 6 the high byte.


    scsi_cmd_write_block = 0x02,
/// WRITE BLOCK ($02)
///    The WRITE BLOCK command writes a a 512-byte block to a device.
/// Description
///    The WRITE BLOCK command writes one 512-byte block to the target device specified in the Unit Number parameter.
///    The block written by this command is read from bank $00 RAM at the address specified in the data buffer pointer.
/// Parameter list
///    • Parameter Count (0): 1 byte, set to 3 for this command
///    • Unit Number (1): 1 byte, in the range $00 - $7E
///    • Data Buffer Pointer (2,3)*: 1 word, data buffer address
///    • Block Number (4.5.6)*: 3 bytes, logical address of target block
/// *These bytes are processed as a single parameter.
///
/// Parameter description
///    • Unit Number: This parameter contains the unit number of the target SCSI device, as loaded into the Zero-Page address$43.
///    • Data Buffer Polnter: This parameter contains the beginning address of the data buffer from which the target block is written.
///     The first byte (byte 2) is the low byte, and the second byte (byte 3) is the high byte of the address.
///    • Block Numbers:"This parameter contains the logical address, on the target device, to which the target block is to be written.
///     Byte 4 is the low byte of this address, byte 5 the middle byte, and byte 6 the high byte.

    
    scsi_cmd_format = 0x03,
/// FORMAT ($03)
///    The FORMAT command prepares all of the blocks on a block device for READIWRITE use.
///    FORMAT does n o t prepare blocks for use by a specific operating' system.
/// Description
///    The FORMAT command prepares all of the blocks on the device specified in the Unit Number parameter for ReadIWrite use.
///    This command is for use on block-type devices only.
///
/// Parameter list
///    • Parameter Count (0): 1 byte, set to 1 for this command
///    • Unit Number (1):' 1 byte, in the range $00 - $7E
/// Parameter description
///    • Unit Number: This parameter contains the unit number of the target SCSI device, as loaded into the Zero-Page address$43.
    
    
    scsi_cmd_control = 0x04,
/// CONTROL ($04)
///    The CONTROL command sends control function information to an external device.
///    With the exception of one code, control codes sent by the CONTROL command are device-specific.
///    Code $00 is a soft resset order; it is not device-specific.
/// Description
///    The CONTROL command writes a 512-byte control block to the device specified in the Unit Number parameter.
///    The control block written to the target device contains a control command and a list of data required by the target device to execute that command.
/// Parameter list
///    • Parameter Count (0): 1 byte, set to 3 for this command
///    • Unit Number (1): 1 byte, in the range $00 - $7E
///    • Control Ust Pointers (2,3)-: 1 word, data buffer low and high address
///    • Control Code (4): 1 byte, in the range $00 - $FF
/// Parameter description
///    • Unit Number: This parameter contains the unit number of the target SCSI device, as loaded into the Zero-Page addres $43.
///         If the Unit Number is set to $00, all devices resident on the SCSI bus are targeted and a special set of control commands is used
///         (see Unit Number $00 Commands, below).
///    • Control List Poinfer:'"This parameter contains the beginning address of the data buffer used to store the control list information required
///         by this command. The first byte (byte 2) is the low byte, and the second byte (byte 3) is the high byte of the address.
///    • Control Code: This parameter contains the hexadecimal code number indicating which control command is being issued.
///         With the exception of the mandatory control commands defined in Table 3-9, control commands are device specific.
///         All Apple II SCSI devices must support the control commands defined in Table 3-9

    
    
    scsi_cmd_init = 0x05,
/// INIT ($05)
///    The INIT command resets the Smartport. During initialization, the Srruutport executes a hard reset on all devices connected to the SCSI bus.
/// Description
///    The !NIT command forces the Smartport to reinitialize itself. All of the devices on the SCSI bus are hard reset, and new device numbers assigned, where necessary.
/// Parameter list
///    • Parameter Count (0): 1 byte, set to 1 for this command
///    • Unit Number (1): 1 byte, set to $00 (Smartport) for this command
/// Parameter description
///    • Unit Number: This parameter targets the Smartport.
    
    
    scsi_cmd_open = 0x06,
/// OPEN ($06)
///    The OPEN command prepares a character device for ReadIWrite operations.
/// Description
///    The OPEN command opens a logical me on the target device for data I/O. This command is used for character devices only.
/// Parameter list
///    • Parameter Count (0): 1 byte, set to 1 for this command
///    • Unit Number (1): 1 byte, in the range $01 - $7E
/// Parameter description
///    • Unit Number: This parameter contains the unit number of the target SCSI character device, as loaded into the Zero-Page address $43.
    
    scsi_cmd_close = 0x07,
/// CLOSE ($07)
///    The CLOSE command informs a character device that a ReadlWrite operation sequence is complete.
/// Description
///    The CLOSE command closes a logical me on the target device after a data I/O sequence is completed. This command is used for character devices only.
/// Parameter list
///    • Parameter Count (O). 1 byte, set to 1 for this command
///    • Unit Number (1). 1 byte, in the range $01 - $7E
/// Parameter description
///    • Unit Number: This parameter contains the unit number of the target SCSI character device, as loaded into the Zero-Page address $43.

    
    scsi_cmd_read = 0x08,
/// READ ($08)
///    The READ command reads a specified number of bytes from a device.
/// Description
///    The READ command reads a specified number of bytes from the target device specified in the Unit Number parameter.
///    The bytes read by this command is written into bank $00 RAM beginning at the address specified in the data buffer pointer.
///    The number of bytes to be read is specified in the byte count parameter
/// Parameter list
///    • Parameter Count (0): 1 byte, set to 4 for this command
///    • Unit Number (1): 1 byte, in the range $01 - $7E
///    • Data Buffer Pointer (2,3)": 1 word, data buffer address
///    • Byte Count (4,5~: 2 bytes, number of bytes to read
///    • Address Pointer (4,5,6)*: 3 bytes, device-specific parameter
/// *These bytes are processed as a single parameter.
/// Parameter description
///    • Unit Number: This parameter contains the unit number of the target SCSI device, as loaded into the Zero-Page address$43.
///    • Data Buffer Pointer: This parameter contains the beginning address of the host data buffer to which the target bytes are written.
///     TIte first byte (byte 2) is the low byte, and the second byte (byte 3) is the high byte of the address.
///    • Byte Count: This parameter contains the number of bytes to read for this command. Byte 4 is the low byte of this number, and byte 5 the high byte.
///    • Address Pointer. This parameter contains the addressing information required by the target device. This parameter is device-specific.
    
    
    scsi_cmd_write = 0x09,
/// WRITE ($09)
///    The WRITE command writes a specified number of bytes to a device.
/// Description
///    The WRITE command writes a specified number of bytes to the target device specified in the Unit Number parameter.
///    The bytes written by this command are read from bank $00 RAM beginning at the address specified in the data buffer pointer.
///    The number of bytes to be written is specified in the byte count parameter.
/// Parameter list
///    • Parameter Count (0): 1 byte, set to 4 for this command
///    • Unit Number (1): 1 byte, in the range $01 - $7E
///    • Data Buffer Pointer (2,3)-: 1 word, data buffer address
///    • Byte Count (4,5Y: 2 bytes, number of bytes to read
///    • Address Pointer (4.5.6)*: 3 bytes, device-specific parameter
/// *These bytes are processed as a single parameter.
/// Parameter description
///    • Unit Number: This parameter contains the unit number of the target SCSI device, as loaded into the Zero-Page address$43.
///    • Data Buffer Pointer: This parameter contains the beginning address of the data buffer from which the target bytes are written.
///     The fust byte (byte 2) is the low byte, and the second byte (byte 3) is the high byte of the address.
///    • Byte Count: This parameter contains the number of bytes to write for this command. Byte 4 is the low byte of this number, and byte 5 the high byte.
///    • Address Polnter: This parameter contains the addressing information required by the target device. This parameter is device-specific
} scsi_command_t;


#endif /* scsi_h */
