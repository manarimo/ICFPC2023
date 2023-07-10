import { useMemo } from "react";
import useProblems from "./useProblems";
import useSolutions, { RawSolution } from "./useSolutions";

export interface Solution {
  name: string;
  score: number;
  solutionPath: string;
  tag?: string;
}

export type SolutionHistoryItem = Solution & {
  diffAbs?: number;
  diffPct?: number;
}

const useBestSolutions = (tag?: string) => {
  const problems = useProblems();
  const solutions = useSolutions(tag);

  const data = useMemo(() => {
    const bestSolutions = new Map<number, RawSolution>();
    const solutionHistory: Record<number, SolutionHistoryItem[]> = {};
    solutions.data?.solutions.forEach((solution) => {
      // Keep best solution
      const current = bestSolutions.get(solution.problemId);
      if (!current || solution.score > current.score) {
        bestSolutions.set(solution.problemId, solution);
      }

      // Add solution history item
      if (solutionHistory[solution.problemId] == undefined) {
        solutionHistory[solution.problemId] = [];
      }
      if (solutionHistory[solution.problemId].length > 0) {
        // Calculate diff
        const arr = solutionHistory[solution.problemId];
        const lastEntry = arr[arr.length - 1];
        lastEntry.diffAbs = lastEntry.score - solution.score;
        lastEntry.diffPct = lastEntry.diffAbs / solution.score * 100;
      }
      solutionHistory[solution.problemId].push({
        name: solution.solverName,
        score: solution.score,
        solutionPath: solution.solutionPath,
        tag: solution.tag,
      });
    });

    return problems.data?.problems.map((problem) => {
      const solution = bestSolutions.get(problem.problemId);
      return {
        ...problem,
        bestSolution: solution
          ? {
            name: solution.solverName,
            score: solution.score,
            solutionPath: solution.solutionPath,
            tag: solution.tag,
          }
          : null,
        history: solutionHistory[problem.problemId] ?? [],
      };
    });
  }, [problems.data?.problems, solutions.data?.solutions]);
  const error = problems.error || solutions.error;
  return {
    error,
    data,
  };
};
export default useBestSolutions;
