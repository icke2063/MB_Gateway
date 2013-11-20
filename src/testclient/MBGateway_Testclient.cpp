/**
 * @file   MBGateway_Testclient.cpp
 * @Author icke
 * @date   22.09.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#include <stdio.h>
#include <string.h>
#include <iostream>
using namespace std;

#include <boost/program_options.hpp>
namespace po = boost::program_options;

// program options
po::options_description desc("Allowed options"); //
po::options_description generic("generic options");
po::options_description io_board("IO Board options"); //
po::variables_map vm;

#include <modbus/modbus.h>
#include <modbus/modbus-tcp.h>

#include <SummerySlave.h>
#include <IOBoardSlave.h>
#include <I2C_IO_BOARD.git/common_AVR/include/IO_handling.h>

void printSummeryList(void);
void printSlaveInfo(uint8_t address, uint8_t id);
void printI2CSlave(uint8_t address);
void printIOBoardSlave(uint8_t address);

// Modbus connection
modbus_t *mb = NULL;
string hostname = "127.0.0.1";
int hostport = 502;
int summery = DEFAULT_SUMMERY_ADDR;

int main(int argc, char *argv[]) {
	int ret;
	uint16_t value;

	puts("MB_Gateway Testclient"); /* prints !!!Hello World!!! */

	// set modbus server options
	generic.add_options()
			("help", "produce help message")
			("scan,?","scan Summery Slave")
			("write,w", "write")
			("read,r", "read")
			("address", po::value<uint16_t>(), "modbus register address")
			("value,v", po::value<uint16_t>(), "value")
			("slave",po::value<uint16_t>(), "virtual slave")
			("hostname",po::value<string>(), "MB_Gateway: IP <string>")
			("hostport",po::value<int>(), "MB_Gateway: Port <int>")
			("summery",	po::value<int>(), "Summery Slave <int>");


	io_board.add_options()
			("p_slaveID,1", "print slave ID")
			("p_version,2","print version")
			("p_count,3", "print io pin count")
			("p_port,4", "print port values")
			("p_tmp,5", "print temporary data")
			("p_i2c,6", "print I2C address")
			("p_perm,7", "print permantent data")
			("p_func,8", "print pin function")
			("p_name,9", "print pin name")
			;

	desc.add(generic).add(io_board);

	//getcmdline
	po::store(po::parse_command_line(argc, argv, desc), vm); //get options
	po::notify(vm);

	//print help
	if (vm.count("help")) {
		cout << desc << "\n";
		return 1;
	}

	//get hostname/hostport
	if (vm.count("hostname")) {
		hostname = vm["hostname"].as<string>();
	}
	if (vm.count("hostport")) {
		hostport = vm["hostport"].as<int>();
	}
	if (vm.count("summery")) {
		summery = vm["summery"].as<int>();
	}

	cout << "try connect to MB_Gateway: " << hostname << "@" << hostport
			<< "\n";
	mb = modbus_new_tcp(hostname.c_str(), hostport);
	ret = modbus_connect(mb);
	printf("modbus_connect:%i\n", ret);

	if (ret >= 0) {
		cout << "Summery Slave@ " << summery << "\n";

		//scan summery slave
		if (vm.count("scan"))
			printSummeryList();

		//read/write modbus register
		if (vm.count("slave") && vm.count("address")) {

			printf("Slave:0x%x\n", vm["slave"].as<uint16_t>());
			printf("Address:0x%x\n", vm["address"].as<uint16_t>());

			modbus_set_slave(mb, vm["slave"].as<uint16_t>());
			if (vm.count("read")) { //read value
				printf("read register\n");
				ret = modbus_read_registers(mb, vm["address"].as<uint16_t>(), 1,
						&value);
				if (ret > 0)
					printf("read register: 0x%x\n", value);
			}
			if (vm.count("write") && vm.count("value")) {
				printf("write register\n");
				printf("value:0x%x\n", vm["value"].as<uint16_t>());
				ret = modbus_write_register(mb, vm["address"].as<uint16_t>(),
						vm["value"].as<uint16_t>());
				if (ret > 0)
					printf("write register ok\n");
			}
		}

	} else {
		cout << "got no Connection " << "\n";
	}

}

void printSummeryList(void) {
	int ret;
	uint16_t value;
	string sandbox = "/-\\";

	if (mb != NULL) {

		for (int i = 0; i < DEFAULT_SUMMERY_COUNT; i++) {
//		for (int i = 0; i < 0x44; i++) {

			modbus_set_slave(mb, summery);
			ret = modbus_read_input_registers(mb, i, 1, &value);
			if (ret > 0) {
				if (value != DEFAULT_SUMMERY_VALUE) {
					cout << "\rSummery Slave[0x" << hex << i << "]: 0x" << hex
							<< value << endl;
					printSlaveInfo(i, value);
					printf("\n");
				} else {
					printf("\r%c", sandbox.c_str()[i % 3]);
					fflush(stdout);
				}
			}
		}
		printf("\r");
		fflush(stdout);
	}
}

void printSlaveInfo(uint8_t address, uint8_t id) {

	switch (id) {
	case 0x2A:
		printIOBoardSlave(address);
		break;
	default:
		printI2CSlave(address);
		break;
	}
}

void printI2CSlave(uint8_t address) {
	int ret;
	uint16_t value;

	if (mb != NULL) {
		modbus_set_slave(mb, address);
		cout << "I2CSlave Memory dump" << endl;

		for (int i = 0; i < 128; i++) {
			ret = modbus_read_registers(mb, i, 1, &value);
			if ((i % 16) == 8)
				printf("  |  ");

			if ((i % 16) == 0) {
				printf("\n");
				printf("%4x || ", i);
			}

			if (ret > 0) {
				printf("%4x ", value);
			} else {
				printf("---- ");
			}
			fflush(stdout);
		}
		printf("\n");
	}
}

void printIOBoardSlave(uint8_t address) {
	int ret;
	uint16_t value[32];
	uint8_t pincount;

	printf("printIOBoardSlave\n");

	if (mb != NULL) {
		modbus_set_slave(mb, address);

		printf("RAM:\n");
		//get ID
		if (ret > 0){
			ret = modbus_read_registers(mb, 0, 1, value);

			if(vm.count("p_slaveID")>0){
				printf("[%i]\tSlaveID:\t\t 0x%x\n", 0, value[0]);
			}
		}
		//get Version
		if(vm.count("p_version")>0){
			ret = modbus_read_registers(mb, VERSION_START / 2,
				((VERSION_LENGTH / 2) + (VERSION_LENGTH % 2)), value);
			value[ret < 32 ? ret : 31] = '\0';
			if (ret > 0){
				printf("[%i]\tVersion:\t\t %s\n", VERSION_START / 2, value);
			}
		}

		//virtual io count
		ret = modbus_read_registers(mb, VIRTUAL_IO_COUNT / 2, 1, value);
		if (ret > 0) {
			pincount = value[0] & 0xFF;
			if(vm.count("p_count")>0){
				printf("[%i]\tvirt. IO count:\t\t 0x%x\n", VIRTUAL_IO_COUNT / 2,pincount);
			}
		}

		//virtual IO port
		if(vm.count("p_port")>0){
			ret = modbus_read_registers(mb, VIRTUAL_IO_START / 2,
				(pincount / 8) + ((pincount % 8) > 0 ? 1 : 0), value);

			for (int i = 0; i < ret; i++) {
				printf("[%i]\tvirt. IO port[0x%x]:\t 0x%x\n",
						(VIRTUAL_IO_START / 2) + i, i, value[i]);
			}
		}
		/// add all tmp data handler
		if(vm.count("p_tmp")>0){
			for (int i = 0; i < pincount; i++) {
				uint16_t handler_address = (VIRTUAL_DATA_START / 2) + (i * 2);
				ret = modbus_read_registers(mb, handler_address, 2, value);
				if (ret > 0){

						printf("[%i]\ttmp_data[0x%x]:\t\t 0x%x\n", handler_address, i,
							value[i] & 0xFF);
				}
			}
		}

		printf("EEPROM:\n");
		if(vm.count("p_i2c")>0){
			//get I2C Addi
			ret = modbus_read_registers(mb, I2C_BUFFER_SIZE / 2, 1, value);
			if (ret > 0) {
				printf("[%i]\tI2C Addr:\t\t 0x%x\n", I2C_BUFFER_SIZE / 2,
						value[0]);

			}
		}
		/// read perm data handler
		if(vm.count("p_perm")>0){
			for (int i = 0; i < pincount; i++) {
				uint16_t handler_address = ((I2C_BUFFER_SIZE + EEPROM_DATA_START)
						/ 2) + (i * 2);
				ret = modbus_read_registers(mb, handler_address, 2, value);
				if (ret > 0){
					printf("[%i]\tperm_data[0x%x]:\t\t 0x%x\n", handler_address, i,
							value[i] & 0xFF);
				}
			}
		}

		/// add all function handler
		if(vm.count("p_func")>0){
			for (int i = 0; i < pincount; i++) {
				uint16_t handler_address = (I2C_BUFFER_SIZE / 2)
						+ (EEPROM_FUNC_START / 2) + i;
				ret = modbus_read_registers(mb, handler_address, 1, value);
				if (ret > 0) {
					printf("[%i]\tFunc[0x%x;0x%x]:\t\t 0x%x ", handler_address,
							i / 8, i % 8, value[0] & 0xFF);

					switch (value[0] & 0xff) {
					case PIN_DISABLED:
						printf("disabled \n");
						break;
					case PIN_INPUT:
						printf("input \n");
						break;
					case PIN_SWITCH:
						printf("switch \n");
						break;
					case PIN_TOGGLE:
						printf("toggle \n");
						break;
					case PIN_PULSE:
						printf("pulse \n");
						break;
					case PIN_S0:
						printf("S0 bus \n");
						break;
					case PIN_UART:
						printf("uart \n");
						break;
					case PIN_OW:
						printf("1-wire \n");
						break;
					case PIN_ADC:
						printf("adc \n");
						break;
					default:
						printf("not supported \n");
						break;
					}
				}
			}
		}
		/// add all name handler
		if (vm.count("p_name") > 0) {
			for (int i = 0; i < pincount; i++) {
				uint16_t handler_address =
						((I2C_BUFFER_SIZE + EEPROM_NAME_START) / 2)
								+ (i * (IO_BOARD_MAX_IO_PIN_NAME_LENGTH / 2));
				ret = modbus_read_registers(mb, handler_address, 8, value);

				printf("[%i]:\t", handler_address, ret);
				if (ret > 0) {
					value[ret] = '\0';
					printf("Name:\t %s \n", value);
				} else {
					printf("no name\n");
				}
			}

		}
	}
}
