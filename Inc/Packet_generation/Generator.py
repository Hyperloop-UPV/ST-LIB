import json 
import re

packet_name= "        StackPacket* %name%;\n"
packet_struct = "   %name% = new StackPacket(%packet_data%);\n   packets[id] = %name%;\n   id++;"
enum_template = "enum class %name%{ \n %values% \n};"

class BoardDescription:    
    def __init__(self,name:str,board:dict):
        self.name = name
        self.id = board["board_id"]
        self.ip = board["board_ip"]
        self.size =0
        i = 0
        self.packets = {}
        for packets in board["packets"]:
            packets_name = re.split(r'_|\.', packets)[0]  
            self.packets[packets_name] = []
            measurement = self._MeasurementFileSearch(packets,board["measurements"])
            with open("Inc/Packet_generation/JSON_ADE/boards/" + name+"/" + board["packets"][i]) as f:
                p= json.load(f)
            with open("Inc/Packet_generation/JSON_ADE/boards/" + name + "/" + measurement) as f:
                m = json.load(f)
            i += 1
            for packet in p["packets"]:
                self.packets[packets_name].append(PacketDescription(packet,m))
                self.size = self.size +1
    @staticmethod            
    def _MeasurementFileSearch(packet:str,measurements:dict):
        packet_name = packet.split('_')[0]
        for measurement in measurements:
            measurement_name = measurement.split('_')[0]
            if packet_name[0] == measurement_name[0]:
                return measurement
        else:
            return measurements[0]
class PacketDescription:
    def __init__(self, packet:dict,measurements:dict):
        self.id =packet["id"]
        self.name = (packet["name"].replace(" ", "_"))
        self.type = packet["type"]
        self.variables = []
        self.measurements = []
        i=0
        for variable in packet["variables"]:
            self.variables.append(variable["name"])
            self.measurements.append(MeasurmentsDescription(measurements,variable["name"]))


class MeasurmentsDescription:
    def __init__(self,measurements:dict, variable:str):
        self.id = variable
        measurement = self._MeasurementSearch(measurements,variable)
        if measurement is None:
            raise Exception("Measurement not found")
        else:
            self.name = measurement["name"]
            self.type = self._unsigned_int_correction(measurement["type"])
            if self.type == "enum":
                self.enum = self._create_enum(measurement)
                self.type = measurement["id"]
                
    @staticmethod
    def _MeasurementSearch(measurements:dict, variable:str):
        for measurment in measurements["measurements"]:
            if measurment["id"] == variable:
                return measurment
        return None
    
    @staticmethod
    def _create_enum(measurement: dict):
        if "enumValues" not in measurement:
            raise ValueError("Measurement does not contain 'enumValues'")

        enum = enum_template.replace("%name%", measurement["id"])
        values = ""
        for value in measurement["enumValues"]:
            values += value + ",\n"
        if values.endswith(",\n"):
            values = values[:-2]
            values += "\n"
        enum = enum.replace("%values%", values.strip())
        return enum
    
    @staticmethod
    def _unsigned_int_correction(type:str):
        aux_type = type[:4]
        if aux_type == "uint":
            type += "_t"
        return type
        

def GenerateEnum(board:BoardDescription):
    Enums = set()
    for packet in board.packets:
        if packet != "orders":
            for packet_instance in board.packets[packet]:
                for measurement in packet_instance.measurements:
                    if hasattr(measurement, "enum"):
                        Enums.add(measurement.enum)
    Enums = list(Enums)
    enums_data = "\n".join(Enums)
    return enums_data
    
def GenerateData(board:BoardDescription):
    Data =set()
    for packet in board.packets:
        if packet != "orders":
            for packet_instance in board.packets[packet]:
                data = ""
                i=0
                data += "uint16_t &idpacket" + str(packet_instance.id) + ","
                for variable in packet_instance.variables:
                    data += (str(packet_instance.measurements[i].type)+" &"+ str(variable) +",")
                    i += 1  
                Data.add(data)
    Data = list(Data)
    if Data and Data[-1].endswith(","):
        Data[-1] = Data[-1][:-1]
    total_data ="".join(Data)
    return total_data
    
def GenerateDataNames(board:BoardDescription,packet_name:str):
    Names =[]
    for packet in board.packets:
        if packet != "orders":
            for packet_instance in board.packets[packet]:
                data = ""
                data += packet_name.replace("%name%",packet_instance.name)
                Names.append(data)
    names_data = "".join(Names)
    return names_data
    
    
def GenerateDataPackets(board:BoardDescription,packet_struct:str):
    Packets =[]
    for packet in board.packets:
        if packet != "orders":
            for packet_instance in board.packets[packet]:
                data = ""
                data +="    idpacket"+str(packet_instance.id)+"," 
                
                for variable in packet_instance.variables:
                    data += (str(variable) +",")
                if data.endswith(","):
                        data = data[:-1]  
                aux = packet_struct
                aux = aux.replace("%name%",packet_instance.name)    
                Packets.append(aux.replace("%packet_data%", data))
                
    packets_data = "\n".join(Packets)
    return packets_data

def Generate_DataPackets_hpp(board_input:str):
    with open("Inc/Packet_generation/JSON_ADE/boards.json") as f:
        boards = json.load(f)

    for board in boards["boards"]:
        with open("Inc/Packet_generation/JSON_ADE/" + (boards["boards"][board])) as f:
            b = json.load(f)
        board_instance = BoardDescription(board, b)
        globals()[board] = board_instance

    board_instance = globals()[board_input]
  
    with open("Inc/Packet_generation/Template.hpp","r") as Input:
        data= Input.read()

    data = data.replace("%board%", board_instance.name)
    data = data.replace("%enums%", GenerateEnum(board_instance))
    data = data.replace("%packetnames%", GenerateDataNames(board_instance,packet_name))
    data = data.replace("%size%", str(board_instance.size))
    data = data.replace("%data%", GenerateData(board_instance))
    data = data.replace("%packets%", GenerateDataPackets(board_instance,packet_struct))
    
    with open("Inc/Packet_generation/DataPackets.hpp","w") as Output:
        Output.write(data)

board = input("Enter board name: ")
while board not in ["VCU","OBCCU","LCU"]:
    print("Board not found, only VCU, OBCCU and LCU are available")
    board = input("Enter board name: ")
Generate_DataPackets_hpp(board)







        