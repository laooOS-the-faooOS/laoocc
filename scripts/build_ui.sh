#!/bin/bash

TOTAL="$1"
COUNT=0
BAR_WIDTH=50

tmp=$(mktemp)

cleanup()
{
    printf '\033[?25h'
    printf '\033[0m'
    rm -f "$tmp"
}

trap cleanup EXIT INT TERM

draw_bar()
{
    local filled empty percent bar rest

    filled=$((COUNT * BAR_WIDTH / TOTAL))
    empty=$((BAR_WIDTH - filled))
    percent=$((COUNT * 100 / TOTAL))

    bar=$(printf '%*s' "$filled" '' | tr ' ' '#')
    rest=$(printf '%*s' "$empty" '')

    printf '\033[s'
    printf '\033[999;1H'
    printf '\033[2K\r'
    printf '\033[1;32m[%s%s] %3d%% (%d/%d)\033[0m' \
        "$bar" "$rest" "$percent" "$COUNT" "$TOTAL"
    printf '\033[u'
}

printf '\033[?25l'

# Run the build into a FIFO.
fifo=$(mktemp -u)
mkfifo "$fifo"

cleanup_fifo()
{
    rm -f "$fifo"
}

trap cleanup_fifo EXIT

make --no-print-directory objects > "$fifo" 2>&1 &
MAKE_PID=$!

while IFS= read -r line
do
    if [ "$line" = "@@PROGRESS@@" ]; then
        COUNT=$((COUNT + 1))
        draw_bar
    else
        printf '%s\n' "$line"
    fi
done < "$fifo"

wait "$MAKE_PID"
STATUS=$?

COUNT=$TOTAL
printf "\n"
draw_bar
printf "
"

rm -f "$fifo"

exit "$STATUS"
