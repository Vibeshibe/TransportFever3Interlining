// Export string references, candidate functions, and their callers for review.
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
import java.util.LinkedHashMap;

public class ExportHookEvidence extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 2) {
            throw new IllegalArgumentException("Output path followed by string addresses required");
        }
        LinkedHashMap<Address, Function> functions = new LinkedHashMap<>();
        try (PrintWriter out = new PrintWriter(Files.newBufferedWriter(Path.of(args[0]), StandardCharsets.UTF_8))) {
            out.println("Program: " + currentProgram.getName());
            out.println("Executable SHA256: " + currentProgram.getExecutableSHA256());
            out.println("Image base: " + currentProgram.getImageBase());
            for (int i = 1; i < args.length; i++) {
                Address address = toAddr(args[i]);
                out.println("\nSTRING ADDRESS " + address);
                for (Reference reference : getReferencesTo(address)) {
                    Function function = getFunctionContaining(reference.getFromAddress());
                    out.println("REFERENCE " + reference + " FUNCTION " + function);
                    if (function != null) functions.put(function.getEntryPoint(), function);
                }
            }
            DecompInterface decompiler = new DecompInterface();
            try {
                if (!decompiler.openProgram(currentProgram)) throw new IllegalStateException(decompiler.getLastMessage());
                for (Function function : functions.values()) {
                    monitor.checkCancelled();
                    out.println("\nFUNCTION " + function.getEntryPoint() + " " + function.getName());
                    ReferenceIterator references = currentProgram.getReferenceManager().getReferencesTo(function.getEntryPoint());
                    while (references.hasNext()) {
                        Reference reference = references.next();
                        out.println("CALLER " + reference + " " + getFunctionContaining(reference.getFromAddress()));
                    }
                    DecompileResults result = decompiler.decompileFunction(function, 45, monitor);
                    if (result.decompileCompleted()) out.println(result.getDecompiledFunction().getC());
                    else out.println("DECOMPILE FAILED: " + result.getErrorMessage());
                    out.flush();
                }
            } finally {
                decompiler.dispose();
            }
        }
        println("Evidence written to " + args[0]);
    }
}
