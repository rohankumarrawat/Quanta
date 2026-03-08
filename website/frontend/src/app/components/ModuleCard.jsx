export function ModuleCard({ icon: Icon, title, description }) {
    return (
        <div className="group bg-card border border-border rounded-xl p-6 hover:border-[#a855f7]/50 transition-all hover:shadow-lg hover:shadow-[#a855f7]/5 cursor-pointer">
            <div className="flex items-start gap-4">
                <div className="w-10 h-10 rounded-lg bg-gradient-to-br from-[#a855f7]/20 to-[#22d3ee]/20 flex items-center justify-center flex-shrink-0 group-hover:scale-110 transition-transform">
                    <Icon className="w-5 h-5 text-[#a855f7]" />
                </div>
                <div className="flex-1 min-w-0">
                    <h4 className="font-semibold text-foreground mb-1">{title}</h4>
                    <p className="text-sm text-muted-foreground leading-relaxed">{description}</p>
                </div>
            </div>
        </div>
    );
}
