//================================================================================================
/// @file serial_can_plugin.hpp
///
/// @brief An interface for using a Serial CAN (like CANable 2.0) device through the Serial CAN driver.
/// @author Martin Anzinger
///
/// @copyright 2025 The Open-Agriculture Developers
//================================================================================================
#ifndef SERIAL_CAN_PLUGIN_HPP
#define SERIAL_CAN_PLUGIN_HPP

#ifdef _MSC_VER
//no Microsoft extensions please!
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif
#endif
#include <iostream>
#include <signal.h>
#include <errno.h>

#if !defined(_WIN32) && !defined(_WIN64)
#include <unistd.h>
#if defined(__APPLE__)
#define SERIAL_PORT  "/dev/tty.usbmodemXXX" // Change this to your actual device name
#elif !defined(__CYGWIN__)
#define SERIAL_PORT  "/dev/ttyUSB0"
#else
#define SERIAL_PORT  "/dev/ttyS3"
#endif
#else
#include <Windows.h>
#define SERIAL_PORT  "\\\\.\\COM4"
int usleep(unsigned int usec);
#endif

#include "SerialCAN_Defines.h"
#include "SerialCAN.h"

#ifndef BAUDRATE
#define BAUDRATE  CANBTR_INDEX_250K
#endif

#include "isobus/hardware_integration/can_hardware_plugin.hpp"
#include "isobus/isobus/can_hardware_abstraction.hpp"
#include "isobus/isobus/can_message_frame.hpp"

namespace isobus
{
	//================================================================================================
	/// @class SerialCANPlugin
	///
	/// @brief A Universal Serial CAN plugin that implements the `CANHardwarePlugin` interface.
	//================================================================================================
	class SerialCANPlugin : public CANHardwarePlugin
	{
	public:
		/// @brief Constructor for the SerialCAN plugin
		/// @param[in] channel The channel to use. See definitions in SerialCAN.h such as `/dev/ttyUSB0` or `COM1`
		explicit SerialCANPlugin(std::string channel);

		/// @brief The destructor for SerialCANPlugin
		virtual ~SerialCANPlugin();

		/// @brief Returns if the connection with the hardware is valid
		/// @returns `true` if connected, `false` if not connected
		bool get_is_valid() const override;

		/// @brief Closes the connection to the hardware
		void close() override;

		/// @brief Connects to the hardware you specified in the constructor's channel argument
		void open() override;

		/// @brief Returns a frame from the hardware (synchronous), or `false` if no frame can be read.
		/// @param[in, out] canFrame The CAN frame that was read
		/// @returns `true` if a CAN frame was read, otherwise `false`
		bool read_frame(isobus::CANMessageFrame &canFrame) override;

		/// @brief Writes a frame to the bus (synchronous)
		/// @param[in] canFrame The frame to write to the bus
		/// @returns `true` if the frame was written, otherwise `false`
		bool write_frame(const isobus::CANMessageFrame &canFrame) override;

		/// @brief Returns the device name the driver is using
		/// @returns The device name the driver is using, such as "/dev/tty.usbmodemXXX"
		std::string get_device_name() const;

		/// @brief Changes the name of the device to use, which only works if the device is not open
		/// @param[in] newName The new name for the device (such as "can0" or "vcan0")
		/// @returns `true` if the name was changed, otherwise `false` (if the device is open this will return false)
		bool set_name(const std::string &newName);

	private:
		CSerialCAN serialCanConection; ///< The handle as defined in the SerialCAN driver API
		std::string name; ///< The device name
		CANAPI_Return_t openResult; ///< Stores the result of the call to begin CAN communication. Used for is_valid check later.
	};
}
#endif // SERIAL_CAN_PLUGIN_HPP
