#!/usr/bin/env python3
import accuBar as accuBar
import drawBreakDown as drawBreakDown
import drawTotalLatency as drawTotalLatency


def listSub(a, b):
    ru = []
    for i in range(len(a)):
        k = a[i] - b[i]
        ru.append(k)
    return ru


def main():
    total = drawTotalLatency.main()
    ru, idx = drawBreakDown.main()
    lsub = listSub(total, ru)
    accuBar.DrawFigure(idx, ([lsub]), ['add lantency'], '', 'latency/us', 'tcomp32_2stage_addLatency', True, '')


if __name__ == "__main__":
    main()
