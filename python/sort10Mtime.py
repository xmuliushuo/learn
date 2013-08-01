str = raw_input();
dict = {}
for i in range(1, 8798):
    str = raw_input()
    dict[str.split(' ' )[0]] = float(str.split(' ')[1])
##print dict[str.split(' ')[0]]
sorteddict = sorted(dict.items(), key = lambda t:t[1])
#print sorteddict
for data in sorteddict:
    print data 
