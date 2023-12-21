import pickle

class Planetinfo:
    def __init__(self, freq, ans, hist) -> None:
        self.groupid = 0
        self.frequent = freq
        self.dayfre = 1 if ans else 0
        self.nightfre = 0 if ans else 1
        self.records = []
        self.records.append(Record(ans, hist))

class Record:
    def __init__(self, ans, hist) -> None:
        self.ans = 1 if ans else 0
        self.recfreq = 1
        self.df = 0
        self.nf = 0
        self.hint = 0
        self.histinfo = hist

def genhisthint(info: Planetinfo) -> int:
    i = 0
    record = info.records[0]
    dn = 0
    if(record.df > record.nf):
        dn = 1
    else:
        dn = 0
    hist = record.histinfo[0][1]
    hist = hist & 0b111111
    histhint = (hist << 2) | (1 << 1) | dn
    record.hint = 1
    return histhint
    
def groupidgen(info: Planetinfo) -> int:
    histhint = 0
    inhash = 0
    dn = 0
    remaind = 0
    remainn = 0
    remain = 0
    if(len(info.records) == 1):
        if(info.dayfre > info.nightfre):
            dn = 1
        else:
            dn = 0
    else:
        histhint = genhisthint(info)
        for record in info.records:
            if(not record.hint):
                remaind += record.df
                remainn += record.nf
        remain = remaind + remainn
        if(remaind > remainn):
            dn = 1
        else:
            dn = 0
        if(remaind/remain > 0.56 or remainn/remain > 0.56): # 55
            inhash = 0
        else:    
            if(len(info.records) > 2): # 2
                inhash = 1
            else:
                inhash = 0
    return (histhint << 2) | (dn << 1) | inhash


def bubbleSort(records: list[Record]):
    n = len(records)
    for i in range(n): 
        for j in range(0, n-i-1):
            if greater(records[j], records[j+1]) :
                records[j], records[j+1] = records[j+1], records[j]
    return records

def bubbleSort2(records: list[Record]):
    n = len(records)
    for i in range(n): 
        for j in range(0, n-i-1):
            if greaterhist(records[j], records[j+1]) :
                records[j], records[j+1] = records[j+1], records[j]
    return records

def same(a: Record, b: Record, difl: int):
    if(not (a.ans == b.ans)):
        return 0
    for i in range(difl):
        if(not (a.histinfo[i][0] == b.histinfo[i][0])):
            return 0
        if(not (a.histinfo[i][1] == b.histinfo[i][1])):
            return 0
    return 1

def samehist(a: Record, b: Record, difl: int):
    for i in range(difl):
        if(not (a.histinfo[i][0] == b.histinfo[i][0])):
            return 0
        if(not (a.histinfo[i][1] == b.histinfo[i][1])):
            return 0
    return 1

def greater(a: Record, b: Record):
    if(a.ans > b.ans):
        return 1
    elif(a.ans < b.ans):
        return 0
    for i in range(len(a.histinfo)):
        if(a.histinfo[i][0] > b.histinfo[i][0]):
            return 1
        elif(a.histinfo[i][0] < b.histinfo[i][0]):
            return 0
        if(a.histinfo[i][1] > b.histinfo[i][1]):
            return 1
        elif(a.histinfo[i][1] < b.histinfo[i][1]):
            return 0
    return 0

def greaterhist(a: Record, b: Record):
    for i in range(len(a.histinfo)):
        if(a.histinfo[i][0] > b.histinfo[i][0]):
            return 1
        elif(a.histinfo[i][0] < b.histinfo[i][0]):
            return 0
        if(a.histinfo[i][1] > b.histinfo[i][1]):
            return 1
        elif(a.histinfo[i][1] < b.histinfo[i][1]):
            return 0
    if(a.ans > b.ans):
        return 1
    elif(a.ans < b.ans):
        return 0
    return 0

class Analyser:
    def __init__(self, data, addr) -> None:
        self.numhist = 8
        self.data = data
        self.hist = [[0, False]] * self.numhist
        self.planets = {}
        self.planetlist = {}
        try:
            with open(addr, "w"):
                pass
        except:
            print("Unable to write file %s" % addr)
            self.addr = ""
        else:
            self.addr = addr

    def analyse(self) -> None:
        # i = 0
        # for id, ans, grp in self.data:
        #     record = self.planets.get(id) # [[frequent], [[time, hist]]] hist=[[time, id]]
        #     if(i == self.numhist):
        #         if(record):
        #             record.frequent += 1
        #             if(ans):
        #                 record.dayfre += 1
        #             else:
        #                 record.nightfre += 1
        #             record.records.append(Record(ans, list(reversed(self.hist))))
        #         else:
        #             self.planets[id] = Planetinfo(1, ans, list(reversed(self.hist)))
        #     else:
        #         i += 1
        #     self.hist.pop(0)
        #     self.hist.append([1 if ans else 0, id])

        # print("Num of planets: %d" % len(self.planets.items()))

        # k = 0
        # for id, info in self.planets.items():
        #     print("\rPlanet %d, %d" % (k, id), end="")
        #     records = info.records
        #     records = bubbleSort(records)
        #     j = 0
        #     while(j < len(records)):
        #         samel = 1
        #         fre = records[j].recfreq
        #         i = j + 1
        #         while(i < len(records)):
        #             if(records[i].ans == records[j].ans and same(records[i], records[j], samel)):
        #                 fre += records[i].recfreq
        #                 records.pop(i)
        #             else:
        #                 break
        #         records[j].histinfo = records[j].histinfo[0:samel]
        #         records[j].recfreq = fre
        #         j += 1

        #     records = bubbleSort2(records)
        #     j = 0
        #     while(j < len(records)):
        #         i = j+1
        #         if(i < len(records) and samehist(records[i], records[j], samel)):
        #             if(records[j].ans == 0):
        #                 records[j].df = records[i].recfreq
        #                 records[j].nf = records[j].recfreq
        #             else:
        #                 records[j].df = records[j].recfreq
        #                 records[j].nf = records[i].recfreq
        #             records[j].recfreq = records[j].df + records[j].nf
        #             records.pop(i)
        #             j += 1
        #         else:
        #             if(records[j].ans == 1):
        #                 records[j].df = records[j].recfreq
        #             else:
        #                 records[j].nf = records[j].recfreq
        #             j += 1
            
        #     records = sorted(records, key=lambda x: x.recfreq, reverse=True)
        #     info.records = records
        #     self.planetlist[id] = info
        #     k += 1
        
        # self.planetlist = {k: v for k, v in sorted(
        #     self.planetlist.items(), key=lambda item: item[1].frequent, reverse=True)}

        # with open('data.pickle', 'wb') as f:
        #     pickle.dump(self.planetlist, f)

        with open('data.pickle', 'rb') as f:
            self.planetlist = pickle.load(f)
        
        i = 0
        inhash = 0
        hinted = 0
        num = 0
        if(self.addr):
            with open(self.addr, "w") as f:
                for id, info in self.planetlist.items():
                    fre = info.frequent
                    freht = 0
                    s = "%d\t%d\t%d\t%d\n" % (id, fre, info.dayfre, info.nightfre)
                    f.write(s)
                    for record in info.records:
                        s = ""
                        freht += record.recfreq
                        s += str(record.recfreq) + '\t' + str(record.df) + '\t' + str(record.nf) + '\t'
                        for hist in record.histinfo:
                            s += str(hist) + ' '
                        s += "\n"
                        f.write(s)
                    info.groupid = groupidgen(info)
                    if(info.groupid & 0x1):
                        if(i == 2048):
                            info.groupid = info.groupid & 0b1111111110
                        else:
                            i += 1
                    inhash += info.groupid & 0x1
                    if(info.groupid & 0x1):
                        num += info.frequent
                    hinted += 1 if (info.groupid >> 2) else 0
        print()
        print(inhash, hinted)
        print(num)

        with open("finalroute.txt", "w") as f:
            f.write("#PlanetID TimeOfDay\n")
            for id, ans, grp in self.data:
                s = "%d\t%s\t%d\n" % (id, "DAY" if ans else "NIGHT", self.planetlist[id].groupid)
                f.write(s)


