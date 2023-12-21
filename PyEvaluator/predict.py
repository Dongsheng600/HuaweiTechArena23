#83.16%
HASHSIZE = 16375 # 16370
HASHLENGTH = 32 # 32

CONFINIT = 10 # 6
CONFINITD = CONFINIT + 1 # +2
JUDGE = CONFINIT + 1 # +1
FREQINIT = 3 # +0

DAYFREQADJUST = 1
NIGHTFREQADJUST = 1
DAYCONFADJUST1 = 1
NIGHTCONFADJUST1 = 1
DAYCONFADJUST2 = 1
NIGHTCONFADJUST2 = 1

class RoboMemory():
    def __init__(self) -> None:
        self.planethash = []
        for i in range(HASHSIZE):
            self.planethash.append([])
            self.planethash[i].append(0)
            self.planethash[i].append(0)
            self.planethash[i].append(0)
            self.planethash[i].append(0)
        self.hist = 0 # 1

        self.key = 0 # 4
        self.tag = 0 # 1
        self.index = 0 # 4
        self.offset = 0 # 1
        self.conf = 0 # 1

        self.highfreq = 0

        self.freq = FREQINIT # 1

        self.notfound = 0

def getformer(val: int) -> int:
    return (val & 0xf0) >> 4

def gettag(val: int) -> int:
    return (val & 0xf8) >> 3

def keygen(id: int) -> int:
    return id >> 1

def taggen(id: int) -> int:
    return (id >> 4) & 0x1f

def judgeconf(conf: int) -> bool:
    if(conf > JUDGE):
        return True
    else:
        return False

def accesshash(RM, key: int, tag: int) -> int:
    index = key
    for i in range(HASHLENGTH):
        hashtag = gettag(RM.planethash[index][0])
        if(hashtag == tag):
            return index
        elif(hashtag == 0):
            return -1
        else:
            index += 1
            if(index == HASHSIZE):
                return -1
    return -1

def matchhist(RM, index: int, hist: int) -> int:
    val = RM.planethash[index]
    offset = 1
    for i in range(3):
        hashhist = getformer(val[offset])
        if(hashhist == hist):
            return offset
        else:
            offset += 1
    return 0

def addhist(RM, index: int) -> int:
    val = RM.planethash[index]
    offset = 1
    for i in range(3):
        hashhist = getformer(val[offset])
        if(hashhist):
            offset += 1
        else:
            return offset
    return 0

def getconf(RM, index: int, offset: int) -> int:
    return RM.planethash[index][offset] & 0xf

def updateconf(RM, hist: int, index: int, offset: int, time: bool):
    if(not offset):
        offset = addhist(RM, index)
        if(offset):
            if(time):
                RM.conf = CONFINIT
            else:
                RM.conf = CONFINIT - 1

    if(offset):
        if(time):
            RM.conf += DAYCONFADJUST1
            if(RM.conf > 15):
                RM.conf = 15
        else:
            RM.conf -= DAYCONFADJUST1
            if(RM.conf < 0):
                RM.conf = 0

        RM.planethash[index][offset] = RM.planethash[index][offset] & 0x00
        RM.planethash[index][offset] = RM.planethash[index][offset] | (hist << 4)
        RM.planethash[index][offset] = RM.planethash[index][offset] | RM.conf
    
    offset = 0
    RM.conf = getconf(RM, index, offset)
    if(time):
        RM.conf += DAYCONFADJUST2
        if(RM.conf > 7):
            RM.conf = 7
    else:
        RM.conf -= NIGHTCONFADJUST2
        if(RM.conf < 0):
            RM.conf = 0
    RM.planethash[index][offset] = RM.planethash[index][offset] & 0xf8
    RM.planethash[index][offset] = RM.planethash[index][offset] | RM.conf

def addrecord(RM, key: int, tag: int, hist: int, time: bool):
    index = key
    found = False
    for i in range(HASHLENGTH):
        hashtag = gettag(RM.planethash[index][0])
        if(hashtag):
            # index = keygen(index)
            index += 1
            if(index == HASHSIZE):
                return
        else:
            found = True
            break
    if(found):
        confinit = (FREQINIT + 1) if time else FREQINIT
        RM.planethash[index][0] = (tag << 3) | confinit
        updateconf(RM, hist, index, 0, time)

def updatefreq(RM, time: bool):
        if(time):
            RM.freq += DAYFREQADJUST
            if(RM.freq > 7):
                RM.freq = 7
        else:
            RM.freq -= NIGHTFREQADJUST
            if(RM.freq < 0):
                RM.freq = 0

def predictTimeOfDayOnNextPlanet(RM: RoboMemory, nextPlanetID: int,
            spaceshipComputerPrediction: bool, nextPlanetGroupTag) -> bool:
    RM.highfreq = nextPlanetGroupTag & 0x1
    RM.key = keygen(nextPlanetGroupTag) * 32
    if(RM.highfreq):
        RM.tag = taggen(nextPlanetID)
        RM.index = accesshash(RM, RM.key, RM.tag)
        if(RM.index == -1):
            RM.notfound += 1
            if(RM.freq > 3):
                return True
            else:
                return False
        else:
            RM.offset = matchhist(RM, RM.index, RM.hist)
            RM.conf = getconf(RM, RM.index, RM.offset)
        return judgeconf(RM.conf)
    else:
        return True if RM.key else False

def observeAndRecordTimeofdayOnNextPlanet(RM: RoboMemory, nextPlanetID: int,
                                          timeofDayOnNextPlanet: bool) -> None:
    if(RM.highfreq):
        if(RM.index == -1):
            addrecord(RM, RM.key, RM.tag, RM.hist, timeofDayOnNextPlanet)
        else:
            updateconf(RM, RM.hist, RM.index, RM.offset, timeofDayOnNextPlanet)

        updatefreq(RM, timeofDayOnNextPlanet)
        RM.hist = (nextPlanetID & 0xf)