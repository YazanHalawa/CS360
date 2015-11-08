function arrayToList (array){
	var list = null;
	for (var i = array.length-1; i >= 0; i--){
		if (i === array.length-1){
			list = prepend(array[i], list);
		} else {
			list = prepend(array[i], list);
		}
	}
	return list;
}

function listToArray (list){
	var array = [];
	var listIterator = list;
	while (listIterator != null){
		array.push(listIterator.value);
		listIterator = listIterator.rest;
	}
	return array;
}

// helper function
function prepend(element, list){
	var newList = {};
	newList.value = element;
	newList.rest = list;
	return newList;
}

// helper function (not recursive)
function nth(list, number){
	var index = 0;
	var listIterator = list;
	while (listIterator != null){
		if (index === number){
			return listIterator.value;
		}
		listIterator = listIterator.rest;
		index++;
	}
	return "undefined"
}

// helper function (recursive)
function nth_rec(list, number){
	if (number === 0)
		return list.value;
	else{
		return nth_rec(list.rest, number-1);
	}
}

console.log(arrayToList([10,20]));
console.log(listToArray(arrayToList([10, 20, 30])));
console.log(prepend(10, prepend(20, null)));
console.log(nth(arrayToList([10, 20, 30]), 1));
console.log(nth_rec(arrayToList([10, 20, 30]), 1));