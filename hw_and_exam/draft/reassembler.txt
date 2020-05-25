reassmble pesudo code

dup test x
holes
many
overlapping
seq x
single x
win


if endIndex!=-1 && index+len(bytes)-1>endIndex
    return


if index==nextindex 
	put into stream
	count = len(bytes)
	byteassembled+=count
	lastProvedSegment = bytes
	nextIndex+=len(bytes)
	
	if nextindex == endIndex
	    byteStream.input_ended()

if index < nextIndex 
    if index+len(bytes) > nextIndex && partOfMatchLastProvedSegment
        //增量
        newStr = substr(bytes,len(bytes)-nextIndex,-1)
        submitSegment(newStr,nextIndex)
    else 
        return

if index>nextIndex
	temporary save to map[index]=[bytes,eof]


while (map[nextIndex] exist) 
    submitSegment(map[nextIndex],nextIndex)
    


