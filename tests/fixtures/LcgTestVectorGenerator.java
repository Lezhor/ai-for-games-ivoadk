import java.io.FileWriter;
import java.io.PrintWriter;
import java.io.IOException;
import java.util.Random;

public class LcgTestVectorGenerator implements AutoCloseable {

    public static void main(String[] args) {
        String fileName = args.length > 0 ? args[0] : "tests/fixtures/lcg_vectors.csv";
        try (LcgTestVectorGenerator gen = new LcgTestVectorGenerator(fileName)) {

            gen.setSeed(12345L);

            for (int i = 0; i < 100; i++) {
                gen.nextInt();
            }
            gen.setSeed(881256321L);
            for (int i = 0; i < 100; i++) {
                gen.nextInt(i + 1);
            }
            gen.setSeed(2);
            for (int i = 0; i < 100; i++) {
                gen.nextLong();
            }

            gen.setSeed(3529L);

            gen.nextLong();
            gen.nextInt(5318);
            gen.nextInt(967321);
            gen.nextLong();
            gen.nextInt(2);
            gen.nextLong();
            gen.nextInt();

        } catch (IOException e) {
            System.err.println("Failed to write test vectors: " + e.getMessage());
        }
    }

    private final Random rng;
    private final PrintWriter out;

    public LcgTestVectorGenerator(String filename) throws IOException {
        this.rng = new Random();
        this.out = new PrintWriter(new FileWriter(filename));
    }

    public void setSeed(long seed) {
        rng.setSeed(seed);
        out.println("seed, " + seed);
    }

    public void nextInt() {
        int result = rng.nextInt();
        out.println("int, " + result);
    }

    public void nextInt(int bound) {
        if (bound <= 0) {
            throw new IllegalArgumentException("bound must be positive");
        }
        int result = rng.nextInt(bound);
        out.println("int-" + bound + ", " + result);
    }

    public void nextLong() {
        long result = rng.nextLong();
        out.println("long, " + result);
    }

    @Override
    public void close() {
        out.close();
        System.out.println("Test vectors written successfully.");
    }
}
