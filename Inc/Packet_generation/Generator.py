import json 
import re
class BoardDescription:    
    def __init__(self,name:str,board:dict):
        self.name = name
        self.id = board["board_id"]
        self.ip = board["board_ip"]
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
                
    def _MeasurementFileSearch(self,packet:str,measurements:dict):
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
        self.name = packet["name"]
        self.type = packet["type"]
        self.variables = []
        self.measurements = []
        for variable in packet["variables"]:
            self.variables.append([variable["name"]])
            self.measurements.append(MeasurmentsDescription(measurements,variable["name"]))


class MeasurmentsDescription:
    def __init__(self,measurements:dict, variable:str):
        self.id = variable
        measurement = self._MeasurementSearch(measurements,variable)
        if measurement is None:
            raise Exception("Measurement not found")
        else:
            self.name = measurement["name"]
            self.type = measurement["type"]
    
    def _MeasurementSearch(self,measurements:dict, variable:str):
        for measurment in measurements["measurements"]:
            if measurment["id"] == variable:
                return measurment
        return None
        


with open("Inc/Packet_generation/JSON_ADE/boards.json") as f:
    boards = json.load(f)

#with open("Inc/Packet_generation/JSON_ADE/boards/LCU/LCU.json") as f:
    #LCU1 = json.load(f)
#LCU = BoardDescription("LCU",LCU1)

#print(LCU.name)
#print(LCU.packets[0].name)
#i=0
#for variable in LCU.packets[0].variables:
    #print(variable)
    #print("\n")
    #print(LCU.packets[0].measurements[i].type)
    #i+=1

for board in boards["boards"]:
    with open("Inc/Packet_generation/JSON_ADE/" + (boards["boards"][board])) as f:
        b = json.load(f)
    board_instance = BoardDescription(board, b)
    globals()[board] = board_instance
    
for packet in OBCCU.packets["control"]:
    print(packet.name)
    for variable in packet.variables:
        print(variable)
    for measurement in packet.measurements:
        print(measurement.type)
    print("\n")

     





        