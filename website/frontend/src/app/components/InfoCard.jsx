export function InfoCard({ icon: Icon, title, description }) {
    return (
        <div className="group bg-card border border-border rounded-xl p-6 hover:border-[#22d3ee]/50 transition-all hover:shadow-lg hover:shadow-[#22d3ee]/5 cursor-pointer">
            <div className="w-12 h-12 rounded-lg bg-gradient-to-br from-[#22d3ee]/20 to-[#a855f7]/20 flex items-center justify-center mb-4 group-hover:scale-110 transition-transform">
                <Icon className="w-6 h-6 text-[#22d3ee]" />
            </div>
            <h3 className="text-lg font-semibold mb-2 text-foreground">{title}</h3>
            <p className="text-sm text-muted-foreground leading-relaxed">{description}</p>
        </div>
    );
}
