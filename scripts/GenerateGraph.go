package main

import (
	"math/rand"
	"time"
	"os"
	"strconv"
	"runtime"
)

func rangeHandler(start, end, V int, m *map[int]map[int]bool, c chan int, done chan bool) {

	for i := start; i <= end; i++ {
		(*m)[i] = make(map[int]bool)
	}

	var ok bool
	var v int
	for u := range c {
		ok = true
		for ok {
			v = rand.Intn(V)
			_, ok = (*m)[u][v]
		}
		(*m)[u][v] = true
	}

	done <- true

}

func min(a, b int) int {
	if a < b {
		return a
	}
	return b
}

func max(a, b int) int {
	if a < b {
		return b
	}
	return a
}

func main() {
	
	runtime.GOMAXPROCS(runtime.NumCPU())

	rand.Seed(time.Now().UTC().UnixNano())

	V, err := strconv.Atoi(os.Args[1])
	if err != nil {
		panic(err)
	}

	E, err := strconv.Atoi(os.Args[2])
	if err != nil {
		panic(err)
	}

	GR, err := strconv.Atoi(os.Args[3])
	if err != nil {
		panic(err)
	}

	fileName := os.Args[4]

	chans := make([]chan int, GR)
	dones := make([]chan bool, GR)
	maps := make([]map[int]map[int]bool, GR)
	stepSize := V/GR

	d := make(chan bool)
	t := 1000
	numFlooders := E/t
	for numFlooders <= 0 {
		t = t/10
		if t==0 {
			numFlooders = 1
		} else {
			numFlooders = E/t
		}
	}


	for i := 0; i < GR; i++ {
		maps[i] = make(map[int]map[int]bool)
		chans[i] = make(chan int)
		dones[i] = make(chan bool)
	}

	flooder := func(T int) {
			for i := 0; i < T; i++ {
				u := rand.Intn(V)
				chans[u/stepSize] <- u
			}
			d <- true
		}
	for i := 1; i<numFlooders; i++ {
		go flooder(t)
	}
	go flooder(E-((numFlooders-1)*t))
	
	for i := 0; i < GR; i++ {
		end := min(V-1, ((i+1)*stepSize)-1)
		go rangeHandler(i*stepSize, end, V, &maps[i], chans[i], dones[i])
	}

	for i := 0; i<numFlooders; i++ {
		<- d
	}

	for i := 0; i < GR; i++ {
		close(chans[i])
	}

	f, _:= os.Create(fileName)
	f.WriteString(strconv.FormatInt(int64(V), 10))
	f.WriteString("\n")
	for i := 0; i < GR; i++ {
		<- dones[i]
		start := i*stepSize
		end := min(V-1, ((i+1)*stepSize)-1)
		for u := start; u <= end; u++ {
			m, _ := maps[i][u]
			f.WriteString(strconv.FormatInt(int64(len(m)), 10))
			for v, _ := range m {
				f.WriteString(" ")
				f.WriteString(strconv.FormatInt(int64(v+1), 10))
			}
			f.WriteString("\n")
		}
	}

	f.Sync()





}