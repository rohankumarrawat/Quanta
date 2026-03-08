import { ArrowBigUp, MessageSquare, Eye } from "lucide-react";
import { useNavigate } from "react-router";

export function QuestionCard({
    _id,
    votes,
    answers,
    views,
    title,
    preview,
    tags,
    username,
    timePosted,
    isAnswered = false,
}) {
    const navigate = useNavigate();

    return (
        <div
            onClick={() => navigate(`/community/${_id}`)}
            className="group bg-card border border-border rounded-xl p-6 hover:border-[#22d3ee]/30 transition-all cursor-pointer"
        >
            <div className="flex gap-6">
                {/* Stats Column */}
                <div className="flex flex-col gap-4 text-center min-w-[80px]">
                    <div className="flex flex-col">
                        <div className="flex items-center justify-center gap-1 text-muted-foreground">
                            <ArrowBigUp className="w-4 h-4" />
                            <span className="text-sm font-medium">{votes}</span>
                        </div>
                        <span className="text-xs text-muted-foreground">votes</span>
                    </div>

                    <div className="flex flex-col">
                        <div className={`flex items-center justify-center gap-1 ${isAnswered ? "text-green-400" : "text-muted-foreground"
                            }`}>
                            <MessageSquare className="w-4 h-4" />
                            <span className="text-sm font-medium">{Array.isArray(answers) ? answers.length : (answers || 0)}</span>
                        </div>
                        <span className="text-xs text-muted-foreground">answers</span>
                    </div>

                    <div className="flex flex-col">
                        <div className="flex items-center justify-center gap-1 text-muted-foreground">
                            <Eye className="w-4 h-4" />
                            <span className="text-sm font-medium">{views}</span>
                        </div>
                        <span className="text-xs text-muted-foreground">views</span>
                    </div>
                </div>

                {/* Content Column */}
                <div className="flex-1 min-w-0">
                    <h3 className="font-semibold text-foreground mb-2 group-hover:text-[#22d3ee] transition-colors">
                        {title}
                    </h3>
                    <p className="text-sm text-muted-foreground mb-4 line-clamp-2">
                        {preview}
                    </p>

                    {/* Tags */}
                    <div className="flex flex-wrap gap-2 mb-4">
                        {tags.map((tag) => (
                            <span
                                key={tag}
                                className="px-2.5 py-1 rounded-md bg-muted/50 text-xs text-muted-foreground hover:bg-muted hover:text-foreground transition-colors"
                            >
                                {tag}
                            </span>
                        ))}
                    </div>

                    {/* Author Info */}
                    <div className="flex items-center gap-2 text-xs text-muted-foreground">
                        <span className="text-foreground font-medium">{username}</span>
                        <span>•</span>
                        <span>{timePosted}</span>
                    </div>
                </div>
            </div>
        </div>
    );
}
