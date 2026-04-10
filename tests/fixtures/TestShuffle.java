import static java.lang.IO.println;

void main() {
    println(toString(shuffle(new int[]{
            1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19
    }, 42)));

    println(toString(shuffle(new int[]{
            51, 232, 163, 112, 6, 90, 241, 23, 5, 2, 82, 125, 185, 212, 32, 50, 0, 85, 5, 196, 211, 168, 9, 63, 85, 21
    }, 957122)));
}

int[] shuffle(int[] array, long seed) {
    Random r = new Random(seed);
    List<Integer> list = new ArrayList<>(IntStream.of(array).boxed().toList());
    Collections.shuffle(list, r);
    return list.stream().mapToInt(i -> i).toArray();
}

String toString(int[] array) {
    // return IntStream.of(array).mapToObj(String::valueOf).reduce("", (a, b) -> a + b + ", ");
    StringBuilder sb = new StringBuilder();
    for (int i = 0; i < array.length; i++) {
        sb.append(array[i]);
        if (i < array.length - 1)
            sb.append(", ");
    }
    return sb.toString();
}
