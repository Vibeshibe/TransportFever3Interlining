// Decompile specified functions without rerunning whole-program analysis.
// @category TPF3Interlining
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceIterator;
import java.io.PrintWriter;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;

public class ExportFunctions extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 2) throw new IllegalArgumentException("Output path followed by function addresses required");
        DecompInterface decompiler = new DecompInterface();
        try (PrintWriter out = new PrintWriter(Files.newBufferedWriter(Path.of(args[0]), StandardCharsets.UTF_8))) {
            if (!decompiler.openProgram(currentProgram)) throw new IllegalStateException(decompiler.getLastMessage());
            out.println("Executable SHA256: " + currentProgram.getExecutableSHA256());
            for (int i = 1; i < args.length; i++) {
                monitor.checkCancelled();
                Address address = toAddr(args[i]);
                Function function = getFunctionAt(address);
                if (function == null) function = getFunctionContaining(address);
                if (function == null) {
                    disassemble(address);
                    function = createFunction(address, null);
                }
                if (function == null) {
                    out.println("NO FUNCTION AT " + address);
                    continue;
                }
                out.println("\nFUNCTION " + function.getEntryPoint() + " " + function.getName());
                ReferenceIterator references = currentProgram.getReferenceManager().getReferencesTo(function.getEntryPoint());
                while (references.hasNext()) {
                    Reference reference = references.next();
                    out.println("CALLER " + reference + " " + getFunctionContaining(reference.getFromAddress()));
                }
                DecompileResults result = decompiler.decompileFunction(function, 60, monitor);
                if (result.decompileCompleted()) out.println(result.getDecompiledFunction().getC());
                else out.println("DECOMPILE FAILED: " + result.getErrorMessage());
                out.flush();
            }
        } finally {
            decompiler.dispose();
        }
        println("Functions written to " + args[0]);
    }
}
