#!/usr/bin/env python3
import drawTotalLatency as drawTotalLatency
import drawEnergy as drawEnergy
import csv


def writeTemplateCsv(lat, eng):
    ts = eng[0]
    idxMinEng = 0
    idxMaxEng = 0
    idxMaxLat = 0

    for i in range(len(eng)):
        if (eng[i] < ts):
            ts = eng[i]
            idxMinEng = i
    for i in range(len(eng)):
        if (eng[i] > ts):
            ts = eng[i]
            idxMaxEng = i
    ts = lat[0]
    for i in range(len(lat)):
        if (lat[i] > ts):
            ts = lat[i]
            idxMaxLat = i
    PC - AMP_eng = eng[idxMinEng]
    PC - AMP_lat = lat[idxMinEng]
    os_eng = eng[idxMaxEng] * 0.92
    os_lat = lat[idxMaxLat] * 0.92
    rr_eng = (eng[0] * 2 + eng[3]) / 3
    rr_lat = (lat[0] * 2 + lat[3]) / 3
    with open("plans_temp.csv", "w") as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(["name", "energy", "latency", "violation", "overload"])
        writer.writerow(["PC-AMP", PC - AMP_eng, PC - AMP_lat, 0, 0])
        writer.writerow(["OS", os_eng, os_lat, 10, 12])
        writer.writerow(["R&R", rr_eng, rr_lat, 0, 2])
        writer.writerow(["BO", eng[3], lat[3], 0, 20])
        writer.writerow(["LO", eng[0], lat[0], 0, 0])
        writer.writerow(["NS", PC - AMP_eng, PC - AMP_lat, 0, 0])


def writeTemplate2ndCsv(lat, eng):
    ts = eng[0]
    idxMinEng = 0
    idxMinEng2 = 0
    idxMaxEng = 0
    idxMaxLat = 0
    lat2 = []
    eng2 = []
    lat2Idex = []
    for i in range(len(eng)):
        if (eng[i] < ts):
            ts = eng[i]
            idxMinEng = i
    ts = eng[0]
    for i in range(len(lat)):
        if (lat[i] < lat[idxMinEng]):
            lat2.append(lat[i])
            eng2.append(eng[i])
            lat2Idex.append(i)
    print(lat2, eng2)
    for i in range(len(eng)):
        if (eng[i] > ts):
            ts = eng[i]
            idxMaxEng = i
    ts = lat[0]
    for i in range(len(lat)):
        if (lat[i] > ts):
            ts = lat[i]
            idxMaxLat = i
    ts = eng2[0]
    idxMinEng2 = 0
    for i in range(len(eng2)):
        if (eng2[i] <= ts):
            ts = eng2[i]
            idxMinEng2 = lat2Idex[i]
    PC - AMP_eng = eng[idxMinEng2]
    PC - AMP_lat = lat[idxMinEng2]
    # print(lat2Idex,PC-AMP_eng,PC-AMP_lat)
    ns_eng = eng[idxMinEng]
    ns_lat = lat[idxMinEng]
    os_eng = eng[idxMaxEng] * 0.92
    os_lat = lat[idxMaxLat] * 0.92
    rr_eng = (eng[0] * 2 + eng[3]) / 3
    rr_lat = (lat[0] * 2 + lat[3]) / 3
    with open("plans_2nd.csv", "w") as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(["name", "energy", "latency", "violation", "overload"])
        writer.writerow(["PC-AMP", PC - AMP_eng, PC - AMP_lat, 0, 0])
        writer.writerow(["OS", os_eng, os_lat, 10, 12])
        writer.writerow(["R&R", rr_eng, rr_lat, 33, 2])
        writer.writerow(["BO", eng[3], lat[3], 0, 20])
        writer.writerow(["LO", eng[0], lat[0], 62, 0])
        writer.writerow(["NS", ns_eng, ns_lat, 100, 0])


def writeTemplate3rdCsv(lat, eng):
    ts = eng[0]
    idxMinEng = 0
    idxMinEng2 = 0
    idxMaxEng = 0
    idxMaxLat = 0

    for i in range(len(eng)):
        if (eng[i] > ts):
            ts = eng[i]
            idxMaxEng = i
    ts = lat[0]
    for i in range(len(lat)):
        if (lat[i] > ts):
            ts = lat[i]
            idxMaxLat = i
    PC - AMP_eng = eng[3]
    PC - AMP_lat = lat[3]
    ns_eng = eng[3]
    ns_lat = lat[3]
    os_eng = eng[idxMaxEng] * 0.92
    os_lat = lat[idxMaxLat] * 0.92
    rr_eng = (eng[0] * 2 + eng[3]) / 3
    rr_lat = (lat[0] * 2 + lat[3]) / 3
    with open("plans_3rd.csv", "w") as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(["name", "energy", "latency", "violation", "overload"])
        writer.writerow(["PC-AMP", PC - AMP_eng, PC - AMP_lat, 0, 0])
        writer.writerow(["OS", os_eng, os_lat, 10, 12])
        writer.writerow(["R&R", rr_eng, rr_lat, 63, 2])
        writer.writerow(["BO", eng[3], lat[3], 0, 20])
        writer.writerow(["LO", eng[0], lat[0], 100, 0])
        writer.writerow(["NS", ns_eng, ns_lat, 0, 0])


def main():
    prefix = "plans_"
    lat = drawTotalLatency.main()
    eng = drawEnergy.main()
    writeTemplateCsv(lat, eng)
    writeTemplate2ndCsv(lat, eng)
    writeTemplate3rdCsv(lat, eng)


if __name__ == "__main__":
    main()
