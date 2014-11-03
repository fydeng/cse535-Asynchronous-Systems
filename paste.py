a =[[1,2,3],[4,5,6],[7,8,9]]
for i,item in enumerate(a):
	if item == [1,2,3]:
		nextItem = a[i+1]
print(nextItem)