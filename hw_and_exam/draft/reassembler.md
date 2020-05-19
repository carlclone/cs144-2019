reassmble pesudo code

dup test x
holes
many
overlapping
seq
single
win

if index==nextindex 
	put into stream
	count = len(bytes)
	byteassembled+=count
	lastProvedSegment = bytes

if index < nextIndex 
    if index+len(bytes) > nextIndex && partOfMatchLastProvedSegment
        //增量
        newStr = substr(bytes,len(bytes)-nextIndex,-1)
        submitSegment(newStr,nextIndex)
    else 
        return

if index>nextIndex
	temporary save to map[index]=bytes






