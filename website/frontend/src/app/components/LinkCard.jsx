import { ExternalLink, Download } from "lucide-react";

export function LinkCard({ title, href, download }) {
    return (
        <a
            href={href}
            download={download}
            className="flex items-center justify-between bg-card border border-border rounded-xl p-4 hover:border-[#22d3ee]/50 transition-all hover:shadow-md hover:shadow-[#22d3ee]/5 group"
        >
            <span className="text-sm font-medium text-foreground">{title}</span>
            {download ? (
                <Download className="w-4 h-4 text-muted-foreground group-hover:text-[#22d3ee] transition-colors" />
            ) : (
                <ExternalLink className="w-4 h-4 text-muted-foreground group-hover:text-[#22d3ee] transition-colors" />
            )}
        </a>
    );
}
