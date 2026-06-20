

"""
git blame --human
Turns your git history into a dramatic human narrative.
"""

import subprocess
import sys
import os
import json
import urllib.request
import urllib.error
import argparse
import textwrap
from datetime import datetime
from collections import defaultdict


#ANSI Colors

RESET   = "\033[0m"
BOLD    = "\033[1m"
DIM     = "\033[2m"
RED     = "\033[38;5;196m"
ORANGE  = "\033[38;5;214m"
YELLOW  = "\033[38;5;226m"
CYAN    = "\033[38;5;87m"
PURPLE  = "\033[38;5;135m"
GRAY    = "\033[38;5;245m"
WHITE   = "\033[38;5;255m"
GREEN   = "\033[38;5;82m"
PINK    = "\033[38;5;213m"


def c(color: str, text: str) -> str:
    return f"{color}{text}{RESET}"


def print_banner():
    banner = r"""
   __ _  _  _     _     _                        
  / _` || || |   | |__ | |  __ _  _ __ ___    ___
 | (_| || || |_  | '_ \| | / _` || '_ ` _ \  / _ \
  \__, ||_||_(_) | |_) | || (_| || | | | | ||  __/
  |___/           |_.__/|_| \__,_||_| |_| |_| \___|
                                                    
  ──  h u m a n  ──────────────────────────────────
"""
    print(c(PURPLE, banner))
    print(c(GRAY, "  Turning your git history into a story worth reading.\n"))


def separator(char="─", width=60, color=GRAY):
    print(c(color, char * width))


#Git Operations

def run_git(args: list[str], cwd: str) -> str:
    result = subprocess.run(
        ["git"] + args,
        cwd=cwd,
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace"
    )
    if result.returncode != 0:
        raise RuntimeError(f"git {' '.join(args)} failed:\n{result.stderr.strip()}")
    return result.stdout.strip()


def get_repo_name(repo_path: str) -> str:
    try:
        remote = run_git(["remote", "get-url", "origin"], repo_path)
        return remote.rstrip("/").split("/")[-1].replace(".git", "")
    except:
        return os.path.basename(os.path.abspath(repo_path))


def get_commits(repo_path: str, limit: int = 50, file_path: str = None) -> list[dict]:
    fmt = "%H|%an|%ae|%ai|%s|%b"
    args = ["log", f"--pretty=format:{fmt}", f"-{limit}"]
    if file_path:
        args += ["--", file_path]

    raw = run_git(args, repo_path)
    if not raw:
        return []

    commits = []
    for line in raw.split("\n"):
        if not line.strip():
            continue
        parts = line.split("|", 5)
        if len(parts) < 5:
            continue
        sha, author, email, date_str, subject = parts[0], parts[1], parts[2], parts[3], parts[4]
        body = parts[5] if len(parts) > 5 else ""
        try:
            date = datetime.fromisoformat(date_str.strip())
        except:
            date = None

        commits.append({
            "sha": sha[:8],
            "author": author,
            "email": email,
            "date": date,
            "date_str": date_str.strip(),
            "subject": subject,
            "body": body.strip(),
        })

    return commits


def get_blame_for_file(repo_path: str, file_path: str) -> list[dict]:
    """Get git blame output for a specific file."""
    try:
        raw = run_git(["blame", "--porcelain", file_path], repo_path)
    except RuntimeError:
        return []

    lines = []
    current = {}
    for line in raw.split("\n"):
        if not line:
            continue
        if line.startswith("\t"):
            current["code"] = line[1:]
            lines.append(current)
            current = {}
        elif len(line) > 40 and line[:40].replace("0", "").replace(" ", "") == "":
            pass
        elif " " not in line[:41]:
            current["sha"] = line[:8]
        elif line.startswith("author "):
            current["author"] = line[7:]
        elif line.startswith("author-time "):
            ts = int(line[12:])
            current["date"] = datetime.fromtimestamp(ts)
        elif line.startswith("summary "):
            current["subject"] = line[8:]

    return lines


def get_stats(repo_path: str) -> dict:
    """Gather interesting repo statistics."""
    stats = {}

    try:
        # Total commits
        stats["total_commits"] = int(run_git(["rev-list", "--count", "HEAD"], repo_path))
    except:
        stats["total_commits"] = 0

    try:
        # Author breakdown
        raw = run_git(["shortlog", "-sn", "HEAD"], repo_path)
        authors = []
        for line in raw.split("\n"):
            if line.strip():
                parts = line.strip().split("\t", 1)
                if len(parts) == 2:
                    authors.append({"count": int(parts[0]), "name": parts[1]})
        stats["authors"] = authors
    except:
        stats["authors"] = []

    try:
        # First and last commit
        first = run_git(["log", "--reverse", "--pretty=format:%ai|%s", "-1"], repo_path)
        last  = run_git(["log", "--pretty=format:%ai|%s", "-1"], repo_path)
        stats["first_commit"] = first
        stats["last_commit"]  = last
    except:
        pass

    try:
        # Most changed files
        raw = run_git(
            ["log", "--pretty=format:", "--name-only", "--diff-filter=M"],
            repo_path
        )
        file_counts = defaultdict(int)
        for f in raw.split("\n"):
            if f.strip():
                file_counts[f.strip()] += 1
        stats["hottest_files"] = sorted(file_counts.items(), key=lambda x: -x[1])[:5]
    except:
        stats["hottest_files"] = []

    try:
        # Time-of-day commit distribution (to find late-night coders)
        raw = run_git(["log", "--pretty=format:%ai"], repo_path)
        hours = defaultdict(int)
        for line in raw.split("\n"):
            if line.strip():
                try:
                    h = int(line.strip()[11:13])
                    hours[h] += 1
                except:
                    pass
        stats["commit_hours"] = dict(hours)
    except:
        stats["commit_hours"] = {}

    return stats


# ─── Claude API ───────────────────────────────────────────────────────────────

def call_claude(prompt: str, api_key: str, mode: str = "story") -> str:
    system_prompts = {
        "story": (
            "You are a literary narrator writing the dramatic human story of a software project "
            "based on its git history. Write with the gravity of a novelist — not a comedian, "
            "not a tech blogger. Find the emotional arc: the ambition, the 3am desperation, "
            "the quiet victories, the mysterious deletions nobody explained. "
            "Use real names from the commits. Reference real commit messages as dialogue or inner monologue. "
            "Write 3-5 paragraphs. No bullet points. Pure narrative prose."
        ),
        "character": (
            "You are a literary critic profiling the developers behind a codebase. "
            "Based on their commit patterns — times, messages, frequency, what they broke and fixed — "
            "write a short psychological portrait of each major contributor. "
            "Be insightful, not mean. Find the humanity. 2-3 paragraphs per person."
        ),
        "file": (
            "You are narrating the life story of a single source code file. "
            "Based on its blame history — who touched it, when, what they said in commit messages — "
            "write its biography. It has been born, modified, perhaps broken and restored. "
            "Give it a personality. 2-4 paragraphs."
        ),
        "roast": (
            "You are a roast comedian at a developer awards ceremony. "
            "Based on this git history, roast the contributors lovingly. "
            "Call out the 3am commits, the 'fix fix fix' messages, the files nobody dares touch. "
            "Be funny but never cruel. 3-5 paragraphs."
        ),
    }

    headers = {
        "Content-Type": "application/json",
        "x-api-key": api_key,
        "anthropic-version": "2023-06-01",
    }

    payload = json.dumps({
        "model": "claude-sonnet-4-6",
        "max_tokens": 1200,
        "system": system_prompts.get(mode, system_prompts["story"]),
        "messages": [{"role": "user", "content": prompt}],
    }).encode()

    req = urllib.request.Request(
        "https://api.anthropic.com/v1/messages",
        data=payload,
        headers=headers,
        method="POST",
    )

    try:
        with urllib.request.urlopen(req, timeout=30) as resp:
            data = json.loads(resp.read().decode())
            return data["content"][0]["text"]
    except urllib.error.HTTPError as e:
        body = e.read().decode()
        raise RuntimeError(f"API error {e.code}: {body}")


# ─── Prompt Builders ──────────────────────────────────────────────────────────

def build_story_prompt(repo_name: str, commits: list[dict], stats: dict) -> str:
    commit_lines = []
    for c in commits[:30]:
        date = c["date"].strftime("%Y-%m-%d %H:%M") if c["date"] else c["date_str"]
        line = f"  [{c['sha']}] {date} | {c['author']} | {c['subject']}"
        if c["body"]:
            line += f"\n         {c['body'][:100]}"
        commit_lines.append(line)

    author_lines = [f"  {a['name']}: {a['count']} commits" for a in stats.get("authors", [])]
    hot_files = [f"  {f} ({n} changes)" for f, n in stats.get("hottest_files", [])]

    # Detect late night commits
    hours = stats.get("commit_hours", {})
    late_night = sum(hours.get(h, 0) for h in range(0, 5))
    total_h = sum(hours.values()) or 1
    late_pct = int(late_night / total_h * 100)

    prompt = f"""
Repository: {repo_name}
Total commits: {stats.get('total_commits', '?')}
Late night commits (midnight–5am): {late_night} ({late_pct}% of all commits)

Contributors:
{chr(10).join(author_lines) or '  (unknown)'}

Most tortured files (changed most often):
{chr(10).join(hot_files) or '  (none found)'}

Recent commit history (newest first):
{chr(10).join(commit_lines)}

Write the human story of this project. What were these people trying to build? 
What does the commit history reveal about their struggles, obsessions, and triumphs?
"""
    return prompt.strip()


def build_file_prompt(repo_name: str, file_path: str, blame_lines: list[dict]) -> str:
    # Sample evenly across the file
    step = max(1, len(blame_lines) // 20)
    sampled = blame_lines[::step][:20]

    entries = []
    for b in sampled:
        if "author" in b and "subject" in b:
            date = b["date"].strftime("%Y-%m-%d") if "date" in b and b["date"] else "?"
            entries.append(f"  Line by {b['author']} on {date}: '{b.get('subject', '')}' | code: {b.get('code', '').strip()[:60]}")

    prompt = f"""
File: {file_path} in repository {repo_name}
Total lines sampled: {len(sampled)} of {len(blame_lines)}

Blame history (who wrote what, when):
{chr(10).join(entries) or '  (empty file)'}

Write the life story of this file. Who created it? Who kept changing it and why?
What does the code suggest about what they were trying to do?
"""
    return prompt.strip()


def build_roast_prompt(repo_name: str, commits: list[dict], stats: dict) -> str:
    hours = stats.get("commit_hours", {})
    late_msgs = [
        c["subject"] for c in commits
        if c["date"] and c["date"].hour in range(0, 5)
    ][:5]

    fix_msgs = [
        c["subject"] for c in commits
        if any(w in c["subject"].lower() for w in ["fix", "bug", "oops", "typo", "mistake", "sorry", "ugh", "again"])
    ][:8]

    authors = stats.get("authors", [])
    prompt = f"""
Repository: {repo_name}
Total commits: {stats.get('total_commits', '?')}

Contributors and their commit counts:
{chr(10).join(f"  {a['name']}: {a['count']} commits" for a in authors)}

3am commit messages (the confession booth):
{chr(10).join('  "' + m + '"' for m in late_msgs) or '  (they slept like normal people, suspicious)'}

Messages that suggest things went wrong:
{chr(10).join('  "' + m + '"' for m in fix_msgs) or '  (impossible, no bugs exist here)'}

Roast these developers. Make it funny and affectionate.
"""
    return prompt.strip()


# ─── Output Formatting ────────────────────────────────────────────────────────

def print_stats_panel(stats: dict, repo_name: str):
    separator(color=PURPLE)
    print(c(BOLD + WHITE, f"  📁 {repo_name}"))
    separator(color=PURPLE)

    total = stats.get("total_commits", 0)
    print(c(CYAN,   f"  Total commits   : ") + c(WHITE, str(total)))

    authors = stats.get("authors", [])
    if authors:
        print(c(CYAN, f"  Contributors    : ") + c(WHITE, str(len(authors))))
        for a in authors[:3]:
            bar_len = int(a["count"] / max(authors[0]["count"], 1) * 20)
            bar = "█" * bar_len + "░" * (20 - bar_len)
            print(f"    {c(GRAY, bar)} {c(YELLOW, a['name'])} {c(DIM, str(a['count']))}")

    hot = stats.get("hottest_files", [])
    if hot:
        print(c(CYAN, f"\n  Most tortured files:"))
        for f, n in hot:
            print(f"    {c(RED, '●')} {c(WHITE, f)} {c(GRAY, f'({n}×)')}")

    hours = stats.get("commit_hours", {})
    if hours:
        late = sum(hours.get(h, 0) for h in range(0, 5))
        if late:
            print(c(CYAN, f"\n  3am commits     : ") + c(RED, f"{late} (someone needs sleep)"))

    separator(color=PURPLE)
    print()


def print_narrative(text: str, mode: str):
    mode_labels = {
        "story":     ("📖", "THE STORY", CYAN),
        "character": ("🧠", "CHARACTER STUDY", PINK),
        "file":      ("📄", "FILE BIOGRAPHY", YELLOW),
        "roast":     ("🔥", "THE ROAST", ORANGE),
    }
    icon, label, color = mode_labels.get(mode, ("📖", "NARRATIVE", CYAN))

    print(c(color + BOLD, f"  {icon}  {label}"))
    separator(color=color)
    print()

    width = 70
    for para in text.strip().split("\n\n"):
        wrapped = textwrap.fill(para.strip(), width=width, initial_indent="  ", subsequent_indent="  ")
        print(c(WHITE, wrapped))
        print()

    separator(color=color)


# ─── CLI ──────────────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(
        prog="git-blame-human",
        description="Turn your git history into a story worth reading.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=textwrap.dedent("""
        Modes:
          story      The dramatic arc of your entire project  (default)
          character  Psychological portraits of each contributor
          file       Biography of a single file (requires --file)
          roast      A loving roast of your dev team

        Examples:
          python git_blame_human.py .
          python git_blame_human.py /path/to/repo --mode roast
          python git_blame_human.py . --mode file --file src/main.c
          python git_blame_human.py . --limit 100 --no-stats
        """)
    )
    parser.add_argument("repo", nargs="?", default=".", help="Path to git repo (default: .)")
    parser.add_argument("--mode", choices=["story", "character", "file", "roast"],
                        default="story", help="Narrative mode")
    parser.add_argument("--file", "-f", help="File to focus on (for --mode file)")
    parser.add_argument("--limit", "-n", type=int, default=50,
                        help="Max commits to analyze (default: 50)")
    parser.add_argument("--no-stats", action="store_true", help="Skip the stats panel")
    parser.add_argument("--api-key", help="Anthropic API key (or set ANTHROPIC_API_KEY env var)")

    args = parser.parse_args()

    # ── Resolve repo path
    repo_path = os.path.abspath(args.repo)
    if not os.path.isdir(repo_path):
        print(c(RED, f"  ✗ Not a directory: {repo_path}"))
        sys.exit(1)

    # ── Check it's a git repo
    try:
        run_git(["rev-parse", "--git-dir"], repo_path)
    except RuntimeError:
        print(c(RED, f"  ✗ Not a git repository: {repo_path}"))
        sys.exit(1)

    # ── API key
    api_key = args.api_key or os.environ.get("ANTHROPIC_API_KEY")
    if not api_key:
        print(c(RED, "  ✗ No API key found."))
        print(c(GRAY, "  Set ANTHROPIC_API_KEY or use --api-key <key>"))
        sys.exit(1)

    # ── Validate file mode
    if args.mode == "file" and not args.file:
        print(c(RED, "  ✗ --mode file requires --file <path>"))
        sys.exit(1)

    print_banner()

    # ── Collect data
    print(c(GRAY, "  Analyzing repository..."), end="", flush=True)
    repo_name = get_repo_name(repo_path)
    commits   = get_commits(repo_path, limit=args.limit, file_path=args.file if args.mode == "file" else None)
    stats     = get_stats(repo_path)
    print(c(GREEN, " done"))

    if not commits:
        print(c(ORANGE, "  ⚠ No commits found."))
        sys.exit(0)

    # ── Stats panel
    if not args.no_stats:
        print()
        print_stats_panel(stats, repo_name)

    # ── Build prompt
    if args.mode == "file":
        print(c(GRAY, f"  Reading blame for {args.file}..."), end="", flush=True)
        blame = get_blame_for_file(repo_path, args.file)
        print(c(GREEN, " done"))
        prompt = build_file_prompt(repo_name, args.file, blame)
    elif args.mode == "roast":
        prompt = build_roast_prompt(repo_name, commits, stats)
    else:
        prompt = build_story_prompt(repo_name, commits, stats)

    # ── Call Claude
    print(c(GRAY, "  Generating narrative..."), end="", flush=True)
    try:
        narrative = call_claude(prompt, api_key, mode=args.mode)
        print(c(GREEN, " done\n"))
    except RuntimeError as e:
        print(c(RED, f"\n  ✗ API call failed: {e}"))
        sys.exit(1)

    # ── Print result
    print_narrative(narrative, mode=args.mode)
    print(c(DIM, f"  Analyzed {len(commits)} commits · {stats.get('total_commits', '?')} total · {len(stats.get('authors', []))} contributors\n"))


if __name__ == "__main__":
    main()
