function range(start, end, step) {
	if (step === null)
		step = 1;
	var numsInRange = [];
	if (end < start){
		for (var i = start; i >= end; i+= step){
			numsInRange.push(i);
		}
	}
	else {
		for (var i = start; i <= end; i+= step){
			numsInRange.push(i);
		}
	}
	return numsInRange;
}

function sum (myArray){
	var sum = 0;
	for (var i = 0; i < myArray.length; i++){
		sum += myArray[i];
	}
	return sum;
}

console.log(sum(range(1,10)));
console.log(range(1,10,2));
console.log(range(5,2,-1));