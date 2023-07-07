import useSWR from "swr";

const useSolution = (solutionId: string) => {
  const url = `/solutions/${solutionId}.json`;
  const fetcher = async (url: string) => {
    const response = await fetch(url);
    const body = await response.json();
    return body as {
      placements: {
        x: number;
        y: number;
      }[];
    };
  };
  return useSWR(url, fetcher);
};

export default useSolution;
