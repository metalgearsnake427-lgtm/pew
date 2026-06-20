mod git;
mod emotion;
mod narrative;
mod display;

use clap::Parser;
use colored::*;
use std::process;

#[derive(Parser, Debug)]
#[command(
    name = "git-blame-human",
    about = "Turns your git history into a human story — zero AI, zero config",
    after_help = "\
EXAMPLES:
    git-blame-human .
    git-blame-human /path/to/repo
    git-blame-human . --limit 200
    git-blame-human . --no-graph
"
)]
struct Args {
    /// Path to git repository
    #[arg(default_value = ".")]
    repo: String,

    /// Number of commits to analyze
    #[arg(long, short, default_value_t = 100)]
    limit: usize,

    /// Skip the emotion arc graph
    #[arg(long, default_value_t = false)]
    no_graph: bool,

    /// Skip character profiles
    #[arg(long, default_value_t = false)]
    no_chars: bool,
}

fn main() {
    let args = Args::parse();

    let repo_path = std::fs::canonicalize(&args.repo).unwrap_or_else(|_| {
        eprintln!("{}", format!("  ✗ Path not found: {}", args.repo).red());
        process::exit(1);
    });

    if !git::is_git_repo(&repo_path) {
        eprintln!("{}", format!("  ✗ Not a git repository: {}", repo_path.display()).red());
        process::exit(1);
    }

    display::print_banner();

    eprint!("{}", "  Reading git history...".dimmed());
    let stats = git::collect(&repo_path, args.limit);
    eprintln!("{}", " done".green());

    if stats.commits.is_empty() {
        eprintln!("{}", "  ⚠ No commits found.".yellow());
        process::exit(0);
    }

    let emotions: Vec<emotion::Emotion> = stats.commits.iter()
        .map(emotion::detect)
        .collect();

    eprint!("{}", "  Generating narrative...".dimmed());
    let story = narrative::generate(&stats);
    eprintln!("{}", " done\n".green());

    display::print_stats(&stats);

    if !args.no_graph {
        display::print_emotion_graph(&emotions);
    }

    display::print_story(&story);

    if !args.no_chars {
        display::print_characters(&story.characters);
    }

    display::print_verdict(&story.verdict);

    println!(
        "{}",
        format!("  analyzed {} of {} commits\n", stats.commits.len(), stats.total_commits).dimmed()
    );
}
