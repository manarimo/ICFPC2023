import useSWR from "swr";

const useProblems = () => {
  const url =
    "http://icfpc2023-manarimo-3mrzsd.s3-website-ap-northeast-1.amazonaws.com/snapshot/problems.json";
  const fetcher = async (url: string) => {
    const response = await fetch(url);
    const data = await response.json();
    return data as {
      problems: {
        problemId: number;
        roomHeight: number;
        roomWidth: number;
        numAttendees: number;
        numMusicians: number;
      }[];
    };
  };
  return useSWR(url, fetcher);
};
export default useProblems;
