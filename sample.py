#print("hello, world!");
iteration = 15000
array = [];
for i in range (0,iteration):
    array.append(i);
#sort rev
for i in range (0,iteration):
    for j in range (1,iteration-i):
        if (array[j-1]<array[j]):
            tmp = array[j-1];
            array[j-1] = array[j];
            array[j] = tmp;

#for i in range (0,10):
#    print (array[i]);
