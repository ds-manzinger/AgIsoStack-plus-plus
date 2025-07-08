//================================================================================================
/// @file serial_can_plugin.cpp
///
/// @brief An interface for using a Serial CAN (like CANable 2.0) device through the Serial CAN driver.
/// @author Martin Anzinger
///
/// @copyright 2025 The Open-Agriculture Developers
//================================================================================================

#include "isobus/hardware_integration/serial_line_can_plugin.hpp"
#include "isobus/isobus/can_stack_logger.hpp"

#include <thread>

namespace isobus
{

	SerialCANPlugin::SerialCANPlugin(std::string channel) : 
		name(channel), 
		openResult(CSerialCAN::NoError),
		serialCanConection(CSerialCAN())
	{

		CANAPI_OpMode_t opMode = {};
		opMode.byte = CANMODE_DEFAULT;
		


		CSerialCAN::EChannelState state = CSerialCAN::ChannelNotAvailable;
		if (CSerialCAN::NoError != (openResult = CSerialCAN::ProbeChannel(get_device_name().c_str(), opMode, state))) {
			LOG_CRITICAL("[SerialCAN]: Error trying to probe Serial CAN device at %s with error code %d", get_device_name().c_str(), openResult);
			LOG_CRITICAL("[SerialCAN]: Channel state is %d", state);
			return;
		} else {
			LOG_INFO("[SerialCAN]: Successfully probed Serial CAN device at %s", get_device_name().c_str());
		}

		if (CSerialCAN::ChannelNotAvailable == state) {
			LOG_CRITICAL("[SerialCAN]: Serial CAN device at %s is not available, reason: %d", get_device_name().c_str(), state);
			return;
		}
		
		if (CSerialCAN::NoError != (openResult = serialCanConection.InitializeChannel(get_device_name().c_str(), opMode))) {
			LOG_CRITICAL("[SerialCAN]: Error trying to initialize Serial CAN device at %s with error code %d", get_device_name().c_str(), openResult);

		} else {
			LOG_INFO("[SerialCAN]: Successfully initialized Serial CAN device at %s", get_device_name().c_str());
		}
	}

	SerialCANPlugin::~SerialCANPlugin()
	{
	}

	bool SerialCANPlugin::get_is_valid() const
	{
		return (CSerialCAN::NoError == openResult);
	}

	std::string SerialCANPlugin::get_device_name() const
	{
		return name;
	}

	void SerialCANPlugin::close()
	{
		if (CSerialCAN::NoError != (openResult = serialCanConection.TeardownChannel())) {
			LOG_CRITICAL("[SerialCAN]: Error trying to close Serial CAN device at %s with error code %d", get_device_name().c_str(), openResult);
		}
	}

	void SerialCANPlugin::open()
	{
		CANAPI_Bitrate_t bitrate = {};
		bitrate.index = BAUDRATE;
		if (CSerialCAN::NoError != (openResult = serialCanConection.StartController(bitrate))) {
			LOG_CRITICAL("[SerialCAN]: Error trying to connect to Serial CAN device at %s with error code %d", get_device_name().c_str(), openResult);
		}
	}

	bool SerialCANPlugin::read_frame(isobus::CANMessageFrame &canFrame)
	{
		bool retVal = false;
		CANAPI_Message_t message;

		if ( CSerialCAN::NoError == (openResult = serialCanConection.ReadMessage(message, CANREAD_INFINITE))) {
			LOG_DEBUG("[SerialCAN]: Read message with ID %X, DLC %i, data: %02X %02X %02X %02X %02X %02X %02X %02X",
			          message.id, message.dlc,
			          message.data[0], message.data[1], message.data[2], message.data[3],
			          message.data[4], message.data[5], message.data[6], message.data[7]);

			canFrame.dataLength = message.dlc;
			memcpy(canFrame.data, message.data, message.dlc);
			canFrame.identifier = message.id;
			canFrame.isExtendedFrame = message.xtd;
			canFrame.timestamp_us = message.timestamp.tv_nsec / 1000;
			retVal = true;
        }
        else 
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		return retVal;
	}

	bool SerialCANPlugin::write_frame(const isobus::CANMessageFrame &canFrame)
	{
		CANAPI_Return_t result;
		CANAPI_Message_t msgCanMessage;

		msgCanMessage.id = canFrame.identifier;
		msgCanMessage.dlc = canFrame.dataLength;
		msgCanMessage.xtd = canFrame.isExtendedFrame;
		memcpy(msgCanMessage.data, canFrame.data, canFrame.dataLength);

		if ( CSerialCAN::NoError != (result = serialCanConection.WriteMessage(msgCanMessage))) {
			LOG_CRITICAL("[SerialCAN]: Error trying to write message to Serial CAN device at %s with error code %d", get_device_name().c_str(), result);
		}	

		return (CSerialCAN::NoError == result);
	}

	bool SerialCANPlugin::set_name(const std::string &newName)
	{
		bool retVal = false;

		if (!get_is_valid())
		{
			name = newName;
			retVal = true;
		}
		return retVal;
	}
}
