export function Badge({ variant = "secondary", children }) {
    const variants = {
        primary:
            "bg-gradient-to-r from-[#22d3ee]/20 to-[#a855f7]/20 text-[#22d3ee] border border-[#22d3ee]/30",
        secondary: "bg-muted text-muted-foreground border border-border",
    };

    return (
        <span
            className={`inline-flex items-center px-2.5 py-0.5 rounded-full text-xs font-medium ${variants[variant]}`}
        >
            {children}
        </span>
    );
}
